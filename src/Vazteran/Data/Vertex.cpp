#include <glm/gtx/hash.hpp>

#include "Vazteran/Math.hpp"
#include "Vazteran/Data/Vertex.hpp"

namespace vzt {
    bool Vertex::operator==(const vzt::Vertex& other) const {
        return position == other.position
               && textureCoordinates == other.textureCoordinates
               && normal == other.normal;
    }

    VkVertexInputBindingDescription Vertex::GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(vzt::Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(vzt::Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(vzt::Vertex, textureCoordinates);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(vzt::Vertex, normal);

        return attributeDescriptions;
    }
}

namespace std {
    size_t hash<vzt::Vertex>::operator()(vzt::Vertex const& vertex) const {
        std::size_t res = 0;
        vzt::HashCombine(res, vertex.position);
        vzt::HashCombine(res, vertex.textureCoordinates);
        vzt::HashCombine(res, vertex.normal);

        return res;
    }
}