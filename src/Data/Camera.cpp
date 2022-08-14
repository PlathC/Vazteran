#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

namespace vzt
{
	Transform fromAabb(const Camera& camera, const AABB& reference)
	{
		Transform transform{};

		const Vec3& maximum = reference.maximum;
		const Vec3& minimum = reference.minimum;

		Transform   result;
		const Vec3  target   = (minimum + maximum) * .5f;
		const float bbRadius = glm::compMax(glm::abs(maximum - target));
		const float distance = bbRadius / std::tan(camera.fov * .5f);

		const glm::vec3 direction = Camera::Front;
		result.position           = target + direction * 2.f * distance;
		// result.lookAt(target);

		return result;
	}

	Mat4 Camera::getProjectionMatrix() const { return glm::perspective(fov, aspectRatio, nearClipping, farClipping); }
	Mat4 Camera::getViewMatrix(const Transform& transform) const
	{
		const Vec3 front = glm::normalize(transform.rotation * Camera::Front);
		const Vec3 up    = glm::normalize(transform.rotation * Camera::Up);
		return glm::lookAt(transform.position, transform.position + front, up);
	}

} // namespace vzt
