#ifndef VZT_UTILS_IOEXR_HPP
#define VZT_UTILS_IOEXR_HPP

#include "vzt/Core/File.hpp"
#include "vzt/Data/Image.hpp"

namespace vzt
{
    Image<float> readEXR(const Path& path);
}

#endif // VZT_UTILS_IOEXR_HPP
