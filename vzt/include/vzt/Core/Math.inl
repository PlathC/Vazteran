#include <glm/gtx/quaternion.hpp>

#include "vzt/Core/Math.hpp"

namespace vzt
{
    Extent3D::Extent3D(uint32_t width, uint32_t height, uint32_t depth) : width(width), height(height), depth(depth) {}
    Extent3D::Extent3D(Extent2D extent) : width(extent.width), height(extent.height), depth(1) {}

    constexpr float toRadians(float degrees) { return degrees * vzt::Pi / 180.f; }
    constexpr float toDegrees(const float radians) { return radians * 180.f / vzt::Pi; }
    inline Mat4     toTransformation(const Vec3& position, const Quat& rotation)
    {
        return glm::translate(glm::toMat4(rotation), position);
    }
} // namespace vzt
