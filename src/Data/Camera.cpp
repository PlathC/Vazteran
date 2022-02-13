#include "Vazteran/Data/Camera.hpp"

namespace vzt
{
	Camera::Camera(const vzt::AABB& referenceBoundingBox, const float fieldOfView, const glm::vec3& up,
	               const float near, const float far, const float screenAspectRatio)
	    : fov(fieldOfView), nearClipping(near), farClipping(far), aspectRatio(screenAspectRatio), upVector(up)
	{
		glm::length_t upIndex = 0;
		if (upVector.y == 1.f)
		{
			upIndex = 1;
		}
		else if (upVector.z == 1.f)
		{
			upIndex = 2;
		}

		const float modelHeight = referenceBoundingBox.maximum()[upIndex] - referenceBoundingBox.minimum()[upIndex];
		const auto  modelCenter = (referenceBoundingBox.maximum() + referenceBoundingBox.minimum()) * .5f;
		const float distance    = modelHeight * .5f / std::tan(fov * .5f);

		position    = modelCenter - glm::vec3(0.f, 1.25f, 0.f) * distance;
		front       = glm::normalize(modelCenter - position);
		m_updateFun = UpdateFirstPerson;
	}

	void Camera::setUpdateFunction(const CameraUpdate updateFun) { m_updateFun = updateFun; }

	void Camera::update(const vzt::Dvec2 deltaCursorPosition)
	{
		if (m_updateFun)
		{
			m_updateFun(*this, deltaCursorPosition);
		}
	}

	void Camera::UpdateFirstPerson(Camera& camera, const vzt::Dvec2 deltaCursorPosition)
	{
		// Adapted from https://learnopengl.com/Getting-started/Camera
		static float    yaw         = 90.f;
		static float    pitch       = 0.f;
		constexpr float sensitivity = .5f;

		yaw += static_cast<float>(deltaCursorPosition.x) * sensitivity;
		pitch += static_cast<float>(deltaCursorPosition.y) * sensitivity;

		pitch = std::max(std::min(pitch, 89.f), -89.f);

		vzt::Vec3 direction;
		direction.x = -std::cos(vzt::toRadians(yaw)) * std::cos(vzt::toRadians(pitch));
		direction.y = std::sin(vzt::toRadians(yaw)) * std::cos(vzt::toRadians(pitch));
		direction.z = std::sin(vzt::toRadians(pitch));

		camera.front = glm::normalize(direction);
	}
} // namespace vzt
