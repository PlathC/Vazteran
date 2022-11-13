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

    struct Extent3D
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth = 1;

        inline Extent3D(uint32_t width, uint32_t height, uint32_t depth = 1);
        inline Extent3D(Extent2D extent);
    };

    template <class Type = float>
    struct Vec2Base
    {
        Type x{};
        Type y{};

        Vec2Base() = default;
        Vec2Base(Type v);
        Vec2Base(Type x, Type y);
    };

    using Vec2  = Vec2Base<float>;
    using Vec2f = Vec2Base<float>;
    using Vec2u = Vec2Base<uint32_t>;
    using Vec2i = Vec2Base<int32_t>;

    template <class Type>
    struct Vec3Base
    {
        Type x{};
        Type y{};
        Type z{};

        Vec3Base() = default;
        Vec3Base(Type v);
        Vec3Base(Type x, Type y, Type z);
    };
    using Vec3  = Vec3Base<float>;
    using Vec3f = Vec3Base<float>;
    using Vec3u = Vec3Base<uint32_t>;
    using Vec3i = Vec3Base<int32_t>;

    template <class Type>
    struct Vec4Base
    {
        Type x{};
        Type y{};
        Type z{};
        Type w{};

        Vec4Base() = default;
        Vec4Base(Type v);
        Vec4Base(Type x, Type y, Type z, Type w);
    };
    using Vec4  = Vec4Base<float>;
    using Vec4f = Vec4Base<float>;
    using Vec4u = Vec4Base<uint32_t>;
    using Vec4i = Vec4Base<int32_t>;
} // namespace vzt

#include "vzt/Core/Math.inl"

#endif // VZT_CORE_MATH_HPP
