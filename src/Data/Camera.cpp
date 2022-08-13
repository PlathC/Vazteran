#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace vzt
{
	Transform fromAabb(const Camera& camera, const AABB& reference)
	{
		Transform transform{};

		const Vec3& maximum = reference.maximum;
		const Vec3& minimum = reference.minimum;

		const float modelHeight = glm::length(maximum.z * Camera::Up) - glm::length(minimum.z * Camera::Up);
		const auto  modelCenter = (maximum + minimum) * .5f;
		const float distance    = modelHeight * .5f / std::tan(camera.fov * .5f);

		Transform result;
		result.position = modelCenter - glm::vec3(0.f, 1.25f, 0.f) * distance;

		const Vec3  toModel  = glm::normalize(result.position - modelCenter);
		const float cosTheta = glm::dot(toModel, Camera::Up);
		Vec3        axis;
		if (glm::abs(cosTheta) > 1.f + 1e-4f)
			axis = Camera::Right;
		else
			axis = glm::cross(Camera::Front, toModel);
		result.rotation = glm::angleAxis(glm::acos(glm::dot(toModel, axis)), axis);

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
