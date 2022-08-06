#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace vzt
{
	Mat4 Camera::getProjectionMatrix() const { return glm::perspective(fov, aspectRatio, nearClipping, farClipping); }
	Mat4 Camera::getViewMatrix(const Transform& transform) const
	{
		const Vec3 front = glm::normalize(transform.rotation * Camera::Front);
		const Vec3 up    = glm::normalize(transform.rotation * Camera::Up);
		return glm::lookAt(transform.position, transform.position + front, up);
	}

} // namespace vzt
