#ifndef VZT_CORE_FILE_HPP
#define VZT_CORE_FILE_HPP

#include <filesystem>

namespace vzt
{
    using Path = std::filesystem::path;
    std::string readFile(const Path& path);
} // namespace vzt

#endif // VZT_CORE_FILE_HPP
