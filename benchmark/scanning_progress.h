#pragma once

#include <atomic>
#include <cstdint>

/**
 * @brief Various pieces of metadata to track file system scan progress.
 */
class ScanningProgress
{
  public:
    /**
     * @brief Resets the scanning progress metadata.
     */
    void Reset() noexcept
    {
        filesScanned.store(0);
        directoriesScanned.store(0);
        bytesProcessed.store(0);
        scanCompleted.store(false);
    }

    std::atomic<std::uintmax_t> filesScanned;
    std::atomic<std::uintmax_t> directoriesScanned;
    std::atomic<std::uintmax_t> bytesProcessed;
    std::atomic<bool> scanCompleted;
};
