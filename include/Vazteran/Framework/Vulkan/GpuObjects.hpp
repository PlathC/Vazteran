#ifndef VAZTERAN_GPUOBJECTS_HPP
#define VAZTERAN_GPUOBJECTS_HPP

#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.h>

#include "Vazteran/Core/Math.hpp"

namespace vzt
{
	struct MaterialInfo
	{
		glm::vec4 ambientColor;
		glm::vec4 diffuseColor;
		glm::vec4 specularColor;

		float shininess;
	};

	struct Transforms
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 viewPosition;
	};
} // namespace vzt

#endif // VAZTERAN_GPUOBJECTS_HPP
