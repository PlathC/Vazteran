#ifndef VZT_UTILS_MESHLOADER_HPP
#define VZT_UTILS_MESHLOADER_HPP

#include "vzt/Core/File.hpp"
#include "vzt/Data/Mesh.hpp"

namespace vzt
{
    Mesh readObj(const Path& path);
}

#endif // VZT_UTILS_MESHLOADER_HPP
