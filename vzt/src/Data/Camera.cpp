#include "vzt/Data/Camera.hpp"

namespace vzt
{
    Mat4 Camera::getProjectionMatrix() const
    {
        Mat4 perspective = glm::perspective(fov, aspectRatio, nearClipping, farClipping);
        perspective[1][1] *= -1;
        return perspective;
    }
    Mat4 Camera::getViewMatrix(const Vec3& position, const Quat& rotation) const
    {
        const Vec3 rFront = glm::normalize(rotation * front);
        const Vec3 rUp    = glm::normalize(rotation * up);
        return glm::lookAt(position, position + rFront, rUp);
    }
} // namespace vzt
