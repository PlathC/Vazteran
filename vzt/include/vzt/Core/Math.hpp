#ifndef VZT_CORE_MATH_HPP
#define VZT_CORE_MATH_HPP

#include <cstdint>

#include <glm/gtc/quaternion.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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

    using Vec2  = glm::vec2;
    using Vec2f = Vec2;
    using Vec2u = glm::uvec2;
    using Vec2i = glm::ivec2;

    using Vec3  = glm::vec3;
    using Vec3f = Vec3;
    using Vec3u = glm::uvec3;
    using Vec3i = glm::ivec3;

    using Vec4  = glm::vec4;
    using Vec4f = Vec4;
    using Vec4u = glm::uvec4;
    using Vec4i = glm::ivec4;

    using Mat3 = glm::mat3;
    using Mat4 = glm::mat4;

    using Quat = glm::quat;

    constexpr float Pi = 3.14159265359f;

    constexpr float toRadians(float degrees);
    constexpr float toDegrees(float radians);

    inline Mat4 toTransformation(const Vec3& position, const Quat& rotation);
} // namespace vzt

#include "vzt/Core/Math.inl"

#endif // VZT_CORE_MATH_HPP
