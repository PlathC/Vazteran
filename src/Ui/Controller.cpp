#include "Vazteran/Ui/Controller.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Transform.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	void updateFirstPerson(const Vec2& deltaCursorPosition, Transform& transform)
	{
		constexpr float sensitivity = 1e-2f;
		transform.rotate(Camera::Up, -static_cast<float>(deltaCursorPosition.x) * sensitivity);
		transform.rotate(Camera::Right, static_cast<float>(deltaCursorPosition.y) * sensitivity);
	}

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

	FirstPersonController::FirstPersonController(Transform* transform) : m_transform(transform) {}

	void FirstPersonController::operator()(const Vec2& deltaCursorPosition)
	{
		updateFirstPerson(deltaCursorPosition, *m_transform);
	}

} // namespace vzt
