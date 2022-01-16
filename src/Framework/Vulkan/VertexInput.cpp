#include <glm/gtx/hash.hpp>

#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Framework/Vulkan/VertexInput.hpp"

namespace vzt
{
	/*bool BlinnPhongVertexInput::operator==(const vzt::BlinnPhongVertexInput &other) const
	{
	    return position == other.position && textureCoordinates == other.textureCoordinates && normal == other.normal;
	}*/

	VkVertexInputBindingDescription BlinnPhongVertexInput::GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding   = 0;
		bindingDescription.stride    = sizeof(vzt::BlinnPhongVertexInput);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> BlinnPhongVertexInput::GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
		    std::vector<VkVertexInputAttributeDescription>(3);

		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset   = offsetof(vzt::BlinnPhongVertexInput, position);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset   = offsetof(vzt::BlinnPhongVertexInput, textureCoordinates);

		attributeDescriptions[2].binding  = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset   = offsetof(vzt::BlinnPhongVertexInput, normal);

		return attributeDescriptions;
	}
} // namespace vzt

namespace std
{
	size_t hash<vzt::BlinnPhongVertexInput>::operator()(vzt::BlinnPhongVertexInput const& vertex) const
	{
		std::size_t res = 0;
		vzt::HashCombine(res, vertex.position);
		vzt::HashCombine(res, vertex.textureCoordinates);
		vzt::HashCombine(res, vertex.normal);

		return res;
	}
} // namespace std
