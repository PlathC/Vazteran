#ifndef VAZTERAN_CAMERA_HPP
#define VAZTERAN_CAMERA_HPP

#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct MainCamera
	{
	};

	struct Transform;
	struct Camera
	{
		Mat4 getProjectionMatrix() const;
		Mat4 getViewMatrix(const Transform& transform) const;

		float fov          = vzt::toRadians(45.f);
		float nearClipping = 0.1f;
		float farClipping  = 100.f;
		float aspectRatio  = 16.f / 9.f;

		static constexpr Vec3 Up    = Vec3(0.f, 0.f, 1.f);
		static constexpr Vec3 Front = Vec3(0.f, 1.f, 0.f);
		static constexpr Vec3 Right = Vec3(1.f, 0.f, 0.f);
	};

} // namespace vzt

#endif // VAZTERAN_CAMERA_HPP
