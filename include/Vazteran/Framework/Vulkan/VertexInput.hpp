#ifndef VAZTERAN_FRAMEWORK_VULKAN_VERTEX_INPUT_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_VERTEX_INPUT_HPP

#include <array>

#include <vulkan/vulkan.h>

#include "Vazteran/Core/Math.hpp"

namespace vzt
{
	using BindingDescription   = VkVertexInputBindingDescription;
	using AttributeDescription = std::vector<VkVertexInputAttributeDescription>;

	struct VertexInputDescription
	{
		BindingDescription   binding;
		AttributeDescription attribute;
	};

	struct BlinnPhongVertexInput
	{
		vzt::Vec3 position;
		vzt::Vec2 textureCoordinates;
		vzt::Vec3 normal;

		static VkVertexInputBindingDescription                GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();
	};
} // namespace vzt

namespace std
{
	template <>
	struct hash<vzt::BlinnPhongVertexInput>
	{
		size_t operator()(vzt::BlinnPhongVertexInput const& vertexInput) const;
	};
} // namespace std

#endif // VAZTERAN_FRAMEWORK_VULKAN_VERTEX_INPUT_HPP
