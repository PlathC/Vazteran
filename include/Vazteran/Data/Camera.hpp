
#ifndef VAZTERAN_CAMERA_HPP
#define VAZTERAN_CAMERA_HPP

#include <functional>

#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Core/Math/Aabb.hpp"

namespace vzt
{

	class Camera
	{
	  public:
		using CameraUpdate = std::function<void(Camera&, vzt::Dvec2 /* deltaCursorPosition */)>;
		static void UpdateFirstPerson(Camera& camera, vzt::Dvec2 deltaCursorPosition);

	  public:
		Camera() = default;
		Camera(const vzt::AABB& referenceBoundingBox, const float fov = vzt::toRadians(45.f),
		       const glm::vec3& up = glm::vec3(0.f, 0.f, 1.f), const float near = 0.1f, const float far = 100.f,
		       const float screenAspectRatio = 16.f / 9.f);

		glm::mat4 getProjectionMatrix() const { return glm::perspective(fov, aspectRatio, nearClipping, farClipping); }
		glm::mat4 getViewMatrix() const { return glm::lookAt(position, position + front, upVector); }

		void setUpdateFunction(const CameraUpdate updateFun);

		void update(const vzt::Dvec2 cursorPosition);

	  public:
		vzt::Vec3 position;
		vzt::Vec3 front;

		float fov          = vzt::toRadians(45.f);
		float nearClipping = 0.1f;
		float farClipping  = 100.f;
		float aspectRatio  = 16.f / 9.f;

		glm::vec3 upVector = glm::vec3(0.f, 0.f, 1.f);

	  private:
		CameraUpdate m_updateFun = UpdateFirstPerson;
	};

} // namespace vzt

#endif // VAZTERAN_CAMERA_HPP
