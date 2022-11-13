#include "vzt/Core/Math.hpp"

namespace vzt
{
    Extent3D::Extent3D(uint32_t width, uint32_t height, uint32_t depth) : width(width), height(height), depth(depth) {}
    Extent3D::Extent3D(Extent2D extent) : width(extent.width), height(extent.height), depth(1) {}

    template <class Type>
    Vec2Base<Type>::Vec2Base(Type x, Type y) : x(x), y(y)
    {
    }

    template <class Type>
    Vec2Base<Type>::Vec2Base(Type v) : x(v), y(v)
    {
    }

    template <class Type>
    Vec3Base<Type>::Vec3Base(Type x, Type y, Type z) : x(x), y(y), z(z)
    {
    }

    template <class Type>
    Vec3Base<Type>::Vec3Base(Type v) : x(v), y(v), z(v)
    {
    }

    template <class Type>
    Vec4Base<Type>::Vec4Base(Type x, Type y, Type z, Type w) : x(x), y(y), z(z), w(w)
    {
    }

    template <class Type>
    Vec4Base<Type>::Vec4Base(Type v) : x(v), y(v), z(v), w(v)
    {
    }
} // namespace vzt
