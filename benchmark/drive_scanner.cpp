#include "drive_scanner.h"

#include "scoped_handle.h"
#include "stopwatch.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4996)
#endif // Win32
#include <boost/asio/post.hpp>
#ifdef WIN32
#pragma warning(pop)
#endif // Win32

#ifdef WIN32
#include <FileApi.h>
#include <WinIoCtl.h>
#endif // Win32

namespace
{
#ifdef WIN32
/**
 * @brief Use the `FindFirstFileW(...)` function to retrieve the file size.
 *
 * The `std::filesystem::file_size(...)` function uses a different native function to get at the
 * file size for a given file, and this function (while probably faster than `FindFirstFileW(...)`)
 * has a tendency to throw. If such exceptional behaviour were to occur, then this function can be
 * used to hopefully still get at the file size.
 *
 * @param path[in]               The path to the troublesome file.
 *
 * @returns The size of the file if it's accessible, and zero otherwise.
 */
std::uintmax_t GetFileSizeUsingWinAPI(const std::filesystem::path& path)
{
    std::uintmax_t fileSize{ 0 };

    WIN32_FIND_DATAW fileData;
    const HANDLE fileHandle = FindFirstFileW(path.wstring().data(), &fileData);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    const auto highWord = static_cast<std::uintmax_t>(fileData.nFileSizeHigh);
    fileSize = (highWord << sizeof(fileData.nFileSizeLow) * 8) | fileData.nFileSizeLow;

    return fileSize;
}
#endif // Win32

/**
 * @brief Helper function to safely wrap the retrieval of a file's size.
 *
 * @param path[in]               The path to the file.
 *
 * @return The size of the file if it's accessible, and zero otherwise.
 */
std::uintmax_t ComputeFileSize(const std::filesystem::path& path) noexcept
{
    assert(!std::filesystem::is_directory(path));

    try {
        return std::filesystem::file_size(path);
    } catch (...) {
#ifdef WIN32
        return GetFileSizeUsingWinAPI(path);
#else // Win32
        return 0ull;
#endif
    }
}

/**
 * @brief Performs a post-processing step that iterates through the tree and computes the size
 * of all directories.
 *
 * @param[in, out] tree          The tree whose nodes need their directory sizes computed.
 */
void ComputeDirectorySizes(Tree<FileInfo>& tree)
{
    for (auto&& node : tree) {
        Tree<FileInfo>::Node* parent = node.GetParent();
        if (!parent) {
            return;
        }

        FileInfo& parentInfo = parent->GetData();
        if (parentInfo.type == FileType::Directory) {
            parentInfo.size += node->size;
        }
    }
}

/**
 * @brief Contructs the root node for the file tree.
 *
 * @param[in] path                The path to the directory that should constitute the root node.
 */
std::shared_ptr<Tree<FileInfo>> CreateTreeAndRootNode(const std::filesystem::path& path)
{
    if (!std::filesystem::is_directory(path)) {
        return nullptr;
    }

    static const std::wstring blankExtension = L"";
    FileInfo fileInfo{ path.wstring(), blankExtension, DriveScanner::UndefinedSize,
                       FileType::Directory };

    return std::make_shared<Tree<FileInfo>>(Tree<FileInfo>(std::move(fileInfo)));
}

#ifdef WIN32
ScopedHandle OpenReparsePoint(const std::filesystem::path& path) noexcept
{
    const auto handle = CreateFileW(
        /* lpFileName = */ path.wstring().c_str(),
        /* dwDesiredAccess = */ GENERIC_READ,
        /* dwShareMode = */ 0,
        /* lpSecurityAttributes = */ nullptr,
        /* dwCreationDisposition = */ OPEN_EXISTING,
        /* dwFlagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        /* hTemplateFile = */ nullptr);

    return ScopedHandle{ handle };
}

bool ReadReparsePoint(const std::wstring& path, std::vector<std::byte>& reparseBuffer) noexcept
{
    const auto handle = OpenReparsePoint(path);
    if (!handle.IsValid()) {
        return false;
    }

    DWORD bytesReturned{ 0 };

    const auto successfullyRetrieved =
        DeviceIoControl(
            /* hDevice = */ static_cast<HANDLE>(handle),
            /* dwIoControlCode = */ FSCTL_GET_REPARSE_POINT,
            /* lpInBuffer = */ nullptr,
            /* nInBufferSize = */ 0,
            /* lpOutBuffer = */ static_cast<LPVOID>(reparseBuffer.data()),
            /* nOutBufferSize = */ static_cast<DWORD>(reparseBuffer.size()),
            /* lpBytesReturned = */ &bytesReturned,
            /* lpOverlapped = */ nullptr) == TRUE;

    return successfullyRetrieved && bytesReturned;
}

bool IsReparseTag(const std::filesystem::path& path, DWORD targetTag) noexcept
{
    static std::vector<std::byte> buffer{ MAXIMUM_REPARSE_DATA_BUFFER_SIZE };

    const auto successfullyRead = ReadReparsePoint(path, buffer);
    return successfullyRead
               ? reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer.data())->ReparseTag == targetTag
               : false;
}

bool IsReparsePoint(const std::filesystem::path& path) noexcept
{
    WIN32_FIND_DATAW findData;
    HANDLE handle = FindFirstFileW(path.native().c_str(), &findData);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    const auto attributes = findData.dwFileAttributes;
    if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        FindClose(handle);
        return true;
    }

    FindClose(handle);
    return false;
}
#endif // Win32

bool IsScannable(const std::filesystem::path& path) noexcept
{
#ifdef WIN32
    return !IsReparsePoint(path);
#elif defined(__linux__)
    return !std::filesystem::is_symlink(path);
#endif // Linux
}
} // namespace

DriveScanner::DriveScanner(const std::filesystem::path& path)
    : m_fileTree{ CreateTreeAndRootNode(path) }, m_rootPath{ path }
{
}

void DriveScanner::ProcessFile(
    const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept
{
    m_progress.filesScanned.fetch_add(1);

    const auto fileSize = ComputeFileSize(path);
    if (fileSize == 0u) {
        return;
    }

    m_progress.bytesProcessed.fetch_add(fileSize);

    FileInfo fileInfo{ path.filename().stem().wstring(), path.filename().extension().wstring(),
                       fileSize, FileType::Regular };

    const std::lock_guard<decltype(m_mutex)> lock{ m_mutex };
    node.AppendChild(std::move(fileInfo));
}

void DriveScanner::ProcessPath(
    const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept
{
    bool isRegularFile = false;
    try {
        // In certain cases, this function can, apparently, raise exceptions, although it
        // isn't entirely clear to me what circumstances need to exist for this to occur:
        isRegularFile = std::filesystem::is_regular_file(path);
    } catch (...) {
        return;
    }

    if (isRegularFile) {
        ProcessFile(path, node);
    } else if (std::filesystem::is_directory(path) && IsScannable(path)) {
        try {
            // In some edge-cases, the Windows operating system doesn't allow anyone to access
            // certain directories, and attempts to do so will result in exceptional behaviour---pun
            // intended. In order to deal with these rare cases, we'll need to rely on a try-catch
            // to keep going. One example of a problematic directory in Windows 7 is: "C:\System
            // Volume Information".
            if (std::filesystem::is_empty(path)) {
                return;
            }
        } catch (...) {
            return;
        }

        static const std::wstring blankExtension = L"";
        FileInfo directoryInfo{ path.filename().wstring(), blankExtension,
                                DriveScanner::UndefinedSize, FileType::Directory };

        std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
        auto* const lastChild = node.AppendChild(std::move(directoryInfo));
        lock.unlock();

        m_progress.directoriesScanned.fetch_add(1);

        AddSubDirectoriesToQueue(path, *lastChild);
    }
}

void DriveScanner::AddSubDirectoriesToQueue(
    const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept
{
    auto itr = std::filesystem::directory_iterator{ path };
    const auto end = std::filesystem::directory_iterator{};

    while (itr != end) {
        boost::asio::post(
            m_threadPool, [&, path = itr->path()]() noexcept { ProcessPath(path, node); });

        ++itr;
    }
}

std::shared_ptr<Tree<FileInfo>> DriveScanner::GetTree()
{
    return m_fileTree;
}

const ScanningProgress& DriveScanner::GetProgress() const
{
    return m_progress;
}

void DriveScanner::Start()
{
    m_progress.Reset();

    boost::asio::post(m_threadPool, [&]() noexcept {
        AddSubDirectoriesToQueue(m_rootPath, *m_fileTree->GetRoot());
    });

    m_threadPool.join();

    ComputeDirectorySizes(*m_fileTree);
    m_progress.scanCompleted.store(true);
}
