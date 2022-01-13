
#ifndef VAZTERAN_VERTEX_HPP
#define VAZTERAN_VERTEX_HPP

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace vzt
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 textureCoordinates;
		glm::vec3 normal;

		bool operator==(const vzt::Vertex &other) const;

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
	};
} // namespace vzt

namespace std
{
	template <> struct hash<vzt::Vertex>
	{
		size_t operator()(vzt::Vertex const &vertex) const;
	};
} // namespace std

#endif // VAZTERAN_VERTEX_HPP
