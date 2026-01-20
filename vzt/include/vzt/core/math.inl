#include <glm/gtx/quaternion.hpp>

#include "vzt/core/math.hpp"

namespace vzt
{
    Extent3D::Extent3D(uint32_t width, uint32_t height, uint32_t depth) : width(width), height(height), depth(depth) {}
    Extent3D::Extent3D(Extent2D extent) : width(extent.width), height(extent.height), depth(1) {}
    Extent3D::operator Extent2D() { return {width, height}; }

    constexpr float toRadians(float degrees) { return degrees * vzt::Pi / 180.f; }
    constexpr float toDegrees(const float radians) { return radians * 180.f / vzt::Pi; }
    inline Mat4     toTransformation(const Vec3& position, const Quat& rotation)
    {
        return glm::translate(glm::toMat4(rotation), position);
    }

    // Reference:
    // https://github.com/SaschaWillems/Vulkan/blob/857f028686c41ee909ec812907cc966af962e339/base/VulkanTools.cpp#L399
    inline std::size_t align(std::size_t size, std::size_t alignment)
    {
        return (size + alignment - 1) & ~(alignment - 1);
    }

} // namespace vzt
