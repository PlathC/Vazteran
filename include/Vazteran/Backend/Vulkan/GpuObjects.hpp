#ifndef VAZTERAN_BACKEND_VULKAN_GPUOBJECTS_HPP
#define VAZTERAN_BACKEND_VULKAN_GPUOBJECTS_HPP

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "Vazteran/Core/Math.hpp"

namespace vzt
{
	struct MaterialInfo
	{
		vzt::Vec4 ambientColor;
		vzt::Vec4 diffuseColor;
		vzt::Vec4 specularColor;

		float shininess;
	};

	struct Transforms
	{
		vzt::Mat4 modelViewMatrix;
		vzt::Mat4 projectionMatrix;
		vzt::Mat4 normalMatrix;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_GPUOBJECTS_HPP
