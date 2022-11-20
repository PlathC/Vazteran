#include "vzt/Data/Camera.hpp"

namespace vzt
{
    Mat4 Camera::getProjectionMatrix() const { return glm::perspective(fov, aspectRatio, nearClipping, farClipping); }
    Mat4 Camera::getViewMatrix(const Vec3& position, const Quat& rotation) const
    {
        const Vec3 front = glm::normalize(rotation * Camera::Front);
        const Vec3 up    = glm::normalize(rotation * Camera::Up);
        return glm::lookAt(position, position + front, up);
    }

} // namespace vzt
