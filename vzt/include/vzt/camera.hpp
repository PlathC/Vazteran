#ifndef VZT_DATA_CAMERA_HPP
#define VZT_DATA_CAMERA_HPP

#include "vzt/core/math.hpp"

namespace vzt
{
    struct Camera
    {
        float fov          = vzt::toRadians(45.f);
        float nearClipping = 1e-1f;
        float farClipping  = 1000.f;
        float aspectRatio  = 16.f / 9.f;

        Vec3 up    = Vec3(0.f, 0.f, 1.f);
        Vec3 front = Vec3(0.f, 1.f, 0.f);
        Vec3 right = Vec3(1.f, 0.f, 0.f);

        Mat4 getProjectionMatrix() const;
        Mat4 getViewMatrix(const Mat4& transformation) const;
        Mat4 getViewMatrix(const Vec3& position, const Quat& rotation) const;
    };
} // namespace vzt

#endif // VZT_DATA_CAMERA_HPP
