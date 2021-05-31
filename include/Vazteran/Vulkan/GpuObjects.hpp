#ifndef VAZTERAN_GPUOBJECTS_HPP
#define VAZTERAN_GPUOBJECTS_HPP

#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.h>

#include "Vazteran/Math.hpp"

namespace vzt {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 textureCoordinates;
        glm::vec3 normal;

        bool operator==(const vzt::Vertex& other) const {
            return position == other.position
                && color == other.color
                && textureCoordinates == other.textureCoordinates
                && normal == other.normal;
        }

        static VkVertexInputBindingDescription GetBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(vzt::Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(vzt::Vertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(vzt::Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(vzt::Vertex, textureCoordinates);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(vzt::Vertex, normal);

            return attributeDescriptions;
        }
    };

    struct ObjectData {
        glm::vec4 ambientColor;
        glm::vec4 diffuseColor;
        glm::vec4 specularColor;

        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec3 viewPosition;
        float shininess;
    };
}

namespace std {
    template<> struct hash<vzt::Vertex> {
        size_t operator()(vzt::Vertex const& vertex) const {
            std::size_t res = 0;
            vzt::HashCombine(res, vertex.position);
            vzt::HashCombine(res, vertex.color);
            vzt::HashCombine(res, vertex.textureCoordinates);
            vzt::HashCombine(res, vertex.normal);

            return res;
        }
    };
}

#endif //VAZTERAN_GPUOBJECTS_HPP
