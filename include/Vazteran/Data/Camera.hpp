
#ifndef VAZTERAN_CAMERA_HPP
#define VAZTERAN_CAMERA_HPP

#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Core/Math/Aabb.hpp"

namespace vzt
{
	struct Camera
	{
		vzt::Vec3 position;
		vzt::Vec3 front;

		float fov          = vzt::ToRadians(45.f);
		float nearClipping = 0.1f;
		float farClipping  = 100.f;
		float aspectRatio  = 16.f / 9.f;

		glm::vec3 upVector = glm::vec3(0.f, 0.f, 1.f);

		glm::mat4 Projection() const { return glm::perspective(fov, aspectRatio, nearClipping, farClipping); }

		glm::mat4 View() const { return glm::lookAt(position, position + front, upVector); }

		static Camera FromBoundingBox(const vzt::AABB& referenceBoundingBox, float fov = vzt::ToRadians(45.f),
		                              glm::vec3 upVector = glm::vec3(0.f, 0.f, 1.f), float nearClipping = 0.1f,
		                              float farClipping = 100.f, float aspectRatio = 16.f / 9.f);
	};
} // namespace vzt

#endif // VAZTERAN_CAMERA_HPP
