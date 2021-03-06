#pragma once

#include <cstdint>
#include <string>

/**
 * @brief The FILE_TYPE enum represents the three basic file types: non-directory files,
 * directories, and symbolic links (which includes junctions).
 */
enum class FileType
{
    Regular,
    Directory,
    Symlink
};

/**
 * @brief The FileInfo struct
 */
struct FileInfo
{
    std::string name;
    std::string extension;

    std::uintmax_t size;

    FileType type;
};
