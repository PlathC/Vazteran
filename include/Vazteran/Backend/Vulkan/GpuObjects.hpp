#ifndef VAZTERAN_BACKEND_VULKAN_GPUOBJECTS_HPP
#define VAZTERAN_BACKEND_VULKAN_GPUOBJECTS_HPP

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct GenericMaterial
	{
		vzt::Vec4 diffuse; // diffuse + shininess

		static GenericMaterial fromMaterial(const vzt::Material& original);
	};

	struct Transforms
	{
		vzt::Mat4 modelViewMatrix;
		vzt::Mat4 projectionMatrix;
		vzt::Mat4 normalMatrix;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_GPUOBJECTS_HPP
