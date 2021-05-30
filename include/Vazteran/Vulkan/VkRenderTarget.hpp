#ifndef VAZTERAN_VKRENDERTARGET_HPP
#define VAZTERAN_VKRENDERTARGET_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Types.hpp"
#include "Vazteran/Vulkan/Buffer.hpp"

namespace vzt {
    class GraphicPipeline;
    class LogicalDevice;

    class VkRenderTarget {
    public:
        VkRenderTarget(vzt::LogicalDevice* logicalDevice, vzt::GraphicPipeline* graphicPipeline,
                       const vzt::Model& model, uint32_t imageCount);

        VkRenderTarget(const VkRenderTarget&) = delete;
        VkRenderTarget& operator=(const VkRenderTarget&) = delete;

        VkRenderTarget(VkRenderTarget&& other) noexcept;
        VkRenderTarget& operator=(VkRenderTarget&& other) noexcept;

        vzt::GraphicPipeline* Pipeline() { return m_graphicPipeline; }
        void Render(VkCommandBuffer commandBuffer, uint32_t imageCount);
        void UpdateDescriptorSet(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer);
        void UpdateUniform(const vzt::Transforms& transform);

        ~VkRenderTarget();

    private:
        uint32_t m_imageCount{};
        LogicalDevice* m_logicalDevice = nullptr;

        std::unique_ptr<VertexBuffer> m_vertexBuffer;
        std::unique_ptr<IndexBuffer> m_indexBuffer;
        std::unordered_map<uint32_t, ImageHandler> m_textureHandlers;
        std::unordered_map<uint32_t, uint32_t> m_uniformRanges;

        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;
        std::vector<vzt::Buffer<vzt::Transforms>> m_uniformBuffers;

        vzt::GraphicPipeline* m_graphicPipeline = nullptr;
    };
}

#endif //VAZTERAN_VKRENDERTARGET_HPP
