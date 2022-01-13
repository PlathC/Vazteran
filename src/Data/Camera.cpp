#include "Vazteran/Data/Camera.hpp"

namespace vzt
{
	Camera Camera::FromBoundingBox(
	    const vzt::AABB &referenceBoundingBox, float fov, glm::vec3 upVector, float nearClipping, float farClipping,
	    float aspectRatio)
	{
		glm::length_t upIndex = 0;
		if (upVector[1] == 1.f)
		{
			upIndex = 1;
		}
		else if (upVector[2] == 1.f)
		{
			upIndex = 2;
		}

		const float modelHeight = referenceBoundingBox.Max()[upIndex] - referenceBoundingBox.Min()[upIndex];
		const auto modelCenter = (referenceBoundingBox.Max() + referenceBoundingBox.Min()) / 2.f;
		const float distance = modelHeight / 2.f / std::tan(fov / 2.f);
		return vzt::Camera{
		    modelCenter - glm::vec3(0., 1.25, 0.) * distance, modelCenter, fov, nearClipping, farClipping, aspectRatio};
	}
} // namespace vzt
