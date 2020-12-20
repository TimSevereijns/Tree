#include "DriveScanner.h"

#include "IgnoreUnused.hpp"
#include "ScopedHandle.h"
#include "Stopwatch.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable : 4996)
#include <boost/asio/post.hpp>
#pragma warning(pop)

#include <FileApi.h>
#include <WinIoCtl.h>

namespace
{
   std::mutex streamMutex;

   /**
    * @brief Use the `FindFirstFileW(...)` function to retrieve the file size.
    *
    * The `std::filesystem::file_size(...)` function uses a different native function
    * to get at the file size for a given file, and this function (while probably faster than
    * `FindFirstFileW(...)`) has a tendency to throw. If such exceptional behaviour were to occur,
    * then this function can be used to hopefully still get at the file size.
    *
    * @param path[in]               The path to the troublesome file.
    *
    * @returns The size of the file if it's accessible, and zero otherwise.
    */
   std::uintmax_t GetFileSizeUsingWinAPI(const std::filesystem::path& path)
   {
      std::uintmax_t fileSize{ 0 };

      WIN32_FIND_DATA fileData;
      const HANDLE fileHandle = FindFirstFileW(path.wstring().data(), &fileData);
      if (fileHandle == INVALID_HANDLE_VALUE)
      {
         return 0;
      }

      const auto highWord = static_cast<std::uintmax_t>(fileData.nFileSizeHigh);
      fileSize = (highWord << sizeof(fileData.nFileSizeLow) * 8) | fileData.nFileSizeLow;

      return fileSize;
   }

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

      try
      {
         return std::filesystem::file_size(path);
      }
      catch (...)
      {
         const std::lock_guard<std::mutex> lock{ streamMutex };
         IgnoreUnused(lock);

         std::wcout << "Falling back on the Win API for: \"" << path.wstring() << "\"" << std::endl;
         return GetFileSizeUsingWinAPI(path);
      }
   }

   /**
    * @brief Removes nodes whose corresponding file or directory size is zero. This is often
    * necessary because a directory may contain only a single other directory within it that is
    * empty. In such a case, the outer directory has a size of zero, but
    * std::filesystem::is_empty will still have reported this directory as being
    * non-empty.
    *
    * @param[in, out] tree           The tree to be pruned.
    */
   void PruneEmptyFilesAndDirectories(Tree<FileInfo>& tree)
   {
      std::vector<Tree<FileInfo>::Node*> toBeDeleted;

      for (auto&& node : tree)
      {
         if (node->size == 0)
         {
            toBeDeleted.emplace_back(&node);
         }
      }

      const auto nodesRemoved = toBeDeleted.size();

      for (auto* node : toBeDeleted)
      {
         node->DeleteFromTree();
      }

      std::cout << "Number of Sizeless Files Removed: " << nodesRemoved << std::endl;
   }

   /**
    * @brief Performs a post-processing step that iterates through the tree and computes the size
    * of all directories.
    *
    * @param[in, out] tree          The tree whose nodes need their directory sizes computed.
    */
   void ComputeDirectorySizes(Tree<FileInfo>& tree)
   {
      for (auto&& node : tree)
      {
         Tree<FileInfo>::Node* parent = node.GetParent();
         if (!parent)
         {
            return;
         }

         FileInfo& parentInfo = parent->GetData();
         if (parentInfo.type == FileType::DIRECTORY)
         {
            parentInfo.size += node->size;
         }
      }
   }

   /**
    * @brief Contructs the root node for the file tree.
    *
    * @param[in] path                The path to the directory that should constitute the root node.
    */
   std::shared_ptr<Tree<FileInfo>>
   CreateTreeAndRootNode(const std::filesystem::path& path)
   {
      if (!std::filesystem::is_directory(path))
      {
         return nullptr;
      }

      FileInfo fileInfo{ path.wstring(),
                         /* extension = */ L"",
                         DriveScanner::SIZE_UNDEFINED,
                         FileType::DIRECTORY };

      return std::make_shared<Tree<FileInfo>>(Tree<FileInfo>(std::move(fileInfo)));
   }

   /**
    * @returns A handle representing the repartse point found at the given path. If
    * the path is not a reparse point, then an invalid handle will be returned instead.
    */
   ScopedHandle OpenReparsePoint(const std::filesystem::path& path)
   {
      const auto handle = CreateFile(
          /* fileName = */ path.wstring().c_str(),
          /* desiredAccess = */ GENERIC_READ,
          /* shareMode = */ 0,
          /* securityAttributes = */ 0,
          /* creationDisposition = */ OPEN_EXISTING,
          /* flagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
          /* templateFile = */ nullptr);

      return ScopedHandle{ handle };
   }

   /**
    * @brief Reads the reparse point found at the given path into the output buffer.
    *
    * @returns True if the path could be read as a reparse point, and false otherwise.
    */
   bool ReadReparsePoint(const std::wstring& path, std::vector<std::byte>& reparseBuffer)
   {
      const auto handle = OpenReparsePoint(path);
      if (!handle.IsValid())
      {
         return false;
      }

      DWORD bytesReturned{ 0 };

      const auto successfullyRetrieved =
          DeviceIoControl(
              /* device = */ handle,
              /* controlCode = */ FSCTL_GET_REPARSE_POINT,
              /* inBuffer = */ NULL,
              /* inBufferSize = */ 0,
              /* outBuffer = */ reinterpret_cast<LPVOID>(reparseBuffer.data()),
              /* outBufferSize = */ static_cast<DWORD>(reparseBuffer.size()),
              /* bytesReturned = */ &bytesReturned,
              /* overlapped = */ 0) == TRUE;

      return successfullyRetrieved && bytesReturned;
   }

   /**
    * @returns True if the given file path matches the given reparse tag, and false otherwise.
    */
   bool IsReparseTag(const std::filesystem::path& path, DWORD targetTag)
   {
      static std::vector<std::byte> buffer{ MAXIMUM_REPARSE_DATA_BUFFER_SIZE };

      const auto successfullyRead = ReadReparsePoint(path, buffer);

      return successfullyRead
                 ? reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer.data())->ReparseTag == targetTag
                 : false;
   }

   /**
    * @returns True if the given file path represents a mount point, and false otherwise.
    *
    * @note Junctions in Windows are considered mount points.
    */
   bool IsMountPoint(const std::filesystem::path& path)
   {
      const auto isMountPoint = IsReparseTag(path, IO_REPARSE_TAG_MOUNT_POINT);

      if (isMountPoint)
      {
         const std::lock_guard<decltype(streamMutex)> lock{ streamMutex };
         std::wcout << L"Found Mount Point: " << path.wstring() << L'\n';
      }

      return isMountPoint;
   }

   /**
    * @returns True if the given file path represents a symlink, and false otherwise.
    */
   bool IsSymlink(const std::filesystem::path& path)
   {
      const auto isSymlink = IsReparseTag(path, IO_REPARSE_TAG_SYMLINK);

      if (isSymlink)
      {
         const std::lock_guard<decltype(streamMutex)> lock{ streamMutex };
         std::wcout << L"Found Symlink: " << path.wstring() << L'\n';
      }

      return isSymlink;
   }

   /**
    * @returns True is the given file path matches any of the listed reparse tag, and false
    * otherwise.
    */
   bool IsUnknownReparsePoint(const std::filesystem::path& path)
   {
      constexpr auto reparseTag =
          IO_REPARSE_TAG_MOUNT_POINT | IO_REPARSE_TAG_HSM | IO_REPARSE_TAG_DRIVE_EXTENDER |
          IO_REPARSE_TAG_HSM2 | IO_REPARSE_TAG_SIS | IO_REPARSE_TAG_WIM | IO_REPARSE_TAG_CSV |
          IO_REPARSE_TAG_DFS | IO_REPARSE_TAG_FILTER_MANAGER | IO_REPARSE_TAG_IIS_CACHE |
          IO_REPARSE_TAG_DFSR | IO_REPARSE_TAG_DEDUP | IO_REPARSE_TAG_APPXSTRM |
          IO_REPARSE_TAG_NFS | IO_REPARSE_TAG_FILE_PLACEHOLDER | IO_REPARSE_TAG_DFM |
          IO_REPARSE_TAG_WOF;

      const auto isRandomReparse = IsReparseTag(path, reparseTag);

      if (isRandomReparse)
      {
         const std::lock_guard<decltype(streamMutex)> lock{ streamMutex };
         std::wcout << L"Found unkown: " << path.wstring() << L'\n';
      }

      return isRandomReparse;
   }

   /**
    * @returns True if the given path represents a reparse point, and false otherwise.
    */
   bool IsReparsePoint(const std::filesystem::path& path)
   {
      const ScopedHandle handle = OpenReparsePoint(path);
      if (!handle.IsValid())
      {
         return false;
      }

      BY_HANDLE_FILE_INFORMATION fileInfo = { 0 };

      const auto successfullyRetrieved = GetFileInformationByHandle(handle, &fileInfo);
      if (!successfullyRetrieved)
      {
         return false;
      }

      return fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
   }
}

DriveScanner::DriveScanner(const std::filesystem::path& path)
    : m_fileTree{ CreateTreeAndRootNode(path) }, m_rootPath{ path }
{
}

void DriveScanner::ProcessFile(
    const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept
{
   const auto fileSize = ComputeFileSize(path);
   if (fileSize == 0u)
   {
      return;
   }

   FileInfo fileInfo{ path.filename().stem().wstring(),
                      path.filename().extension().wstring(),
                      fileSize,
                      FileType::REGULAR };

   const std::lock_guard<decltype(m_mutex)> lock{ m_mutex };
   node.AppendChild(std::move(fileInfo));
}

void DriveScanner::ProcessDirectory(
    const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept
{
   bool isRegularFile = false;
   try
   {
      // In certain cases, this function can, apparently, raise exceptions, although it
      // isn't entirely clear to me what circumstances need to exist for this to occur:
      isRegularFile = std::filesystem::is_regular_file(path);
   }
   catch (...)
   {
      return;
   }

   if (isRegularFile)
   {
      ProcessFile(path, node);
   }
   else if (
       std::filesystem::is_directory(path) &&
       !IsReparsePoint(path))
   {
      try
      {
         // In some edge-cases, the Windows operating system doesn't allow anyone to access certain
         // directories, and attempts to do so will result in exceptional behaviour---pun intended.
         // In order to deal with these rare cases, we'll need to rely on a try-catch to keep going.
         // One example of a problematic directory in Windows 7 is: "C:\System Volume Information".
         if (std::filesystem::is_empty(path))
         {
            return;
         }
      }
      catch (...)
      {
         return;
      }

      FileInfo directoryInfo{ path.filename().wstring(),
                              /* extension = */ L"",
                              DriveScanner::SIZE_UNDEFINED,
                              FileType::DIRECTORY };

      std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
      auto* const lastChild = node.AppendChild(std::move(directoryInfo));
      lock.unlock();

      auto itr = std::filesystem::directory_iterator{ path };
      AddDirectoriesToQueue(itr, *lastChild);
   }
}

void DriveScanner::AddDirectoriesToQueue(
    std::filesystem::directory_iterator& itr, Tree<FileInfo>::Node& node) noexcept
{
   const auto end = std::filesystem::directory_iterator{};
   while (itr != end)
   {
      boost::asio::post(
          m_threadPool, [&, path = itr->path() ]() noexcept { ProcessDirectory(path, node); });

      ++itr;
   }
}

std::shared_ptr<Tree<FileInfo>> DriveScanner::GetTree()
{
   return m_fileTree;
}

void DriveScanner::Start()
{
   Stopwatch<std::chrono::seconds>(
       [&]() noexcept {
          boost::asio::post(m_threadPool, [&]() noexcept {
             auto itr = std::filesystem::directory_iterator{ m_rootPath };
             AddDirectoriesToQueue(itr, *m_fileTree->GetRoot());
          });

          m_threadPool.join();
       },
       "\nScanned Drive in ");

   ComputeDirectorySizes(*m_fileTree);
   PruneEmptyFilesAndDirectories(*m_fileTree);
}
