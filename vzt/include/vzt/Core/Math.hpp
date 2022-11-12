#ifndef VZT_CORE_MATH_HPP
#define VZT_CORE_MATH_HPP

#include <cstdint>

namespace vzt
{
    struct Extent2D
    {
        uint32_t width;
        uint32_t height;
    };

    struct Vec2u
    {
        uint32_t x;
        uint32_t y;
    };

    struct Vec2i
    {
        int32_t x;
        int32_t y;
    };
} // namespace vzt

#endif // VZT_CORE_MATH_HPP
