#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

#pragma warning(push)
#pragma warning(disable : 4996)
#include <boost/asio/thread_pool.hpp>
#pragma warning(pop)

#include "file_info.h"
#include "scanning_progress.h"
#include "tree.h"
#include "win_hack.h"

/**
 * @brief Wrapper around node and path.
 */
struct NodeAndPath
{
    Tree<FileInfo>::Node& node;
    std::filesystem::path path;
};

/**
 * @brief The Drive Scanner class
 */
class DriveScanner
{
  public:
    static constexpr std::uintmax_t UndefinedSize{ 0 };

    explicit DriveScanner(const std::filesystem::path& path);

    /**
     * @brief Kicks off the drive scanning process.
     */
    void Start();

    /**
     * @returns The file tree.
     */
    std::shared_ptr<Tree<FileInfo>> GetTree();

    /**
     * @returns A constant reference to the progress tracker.
     */
    const ScanningProgress& GetProgress() const;

  private:
    /**
     * @brief Helper function to process a single file.
     *
     * @note This function assumes the path is valid and accessible.
     *
     * @param[in] path                The location on disk to scan.
     * @param[in] fileNode            The Node in Tree to append newly discoved files to.
     */
    void ProcessFile(const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept;

    /**
     * @brief Performs a recursive depth-first exploration of the file system.
     *
     * @param[in] path                The location on disk to scan.
     * @param[in] fileNode            The Node in Tree to append newly discoved files to.
     */
    void ProcessPath(const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept;

    /**
     * @brief Adds directories to thread-pool queue.
     *
     * @param[in] path                Path to the directory to iterate over.
     * @param[in] Node                The Node to append the contents of the directory to.
     */
    void AddSubDirectoriesToQueue(
        const std::filesystem::path& path, Tree<FileInfo>::Node& node) noexcept;

    ScanningProgress m_progress;

    std::shared_ptr<Tree<FileInfo>> m_fileTree{ nullptr };

    const std::filesystem::path m_rootPath;

    std::mutex m_mutex;

    boost::asio::thread_pool m_threadPool{ 4 };
};
