#ifndef VZT_DATA_CAMERA_HPP
#define VZT_DATA_CAMERA_HPP

#include "vzt/Core/Math.hpp"

namespace vzt
{
    struct Camera
    {
        static constexpr Vec3 Up    = Vec3(0.f, 0.f, 1.f);
        static constexpr Vec3 Front = Vec3(0.f, 1.f, 0.f);
        static constexpr Vec3 Right = Vec3(1.f, 0.f, 0.f);

        float fov          = vzt::toRadians(45.f);
        float nearClipping = 0.1f;
        float farClipping  = 100.f;
        float aspectRatio  = 16.f / 9.f;

        Mat4 getProjectionMatrix() const;
        Mat4 getViewMatrix(const Mat4& transformation) const;
        Mat4 getViewMatrix(const Vec3& position, const Quat& rotation) const;
    };
} // namespace vzt

#endif // VZT_DATA_CAMERA_HPP
