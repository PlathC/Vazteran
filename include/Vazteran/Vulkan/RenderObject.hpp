#ifndef VAZTERAN_RENDEROBJECT_HPP
#define VAZTERAN_RENDEROBJECT_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Types.hpp"
#include "Vazteran/Vulkan/Buffer.hpp"

namespace vzt {
    class GraphicPipeline;
    class LogicalDevice;

    class RenderObject {
    public:
        RenderObject(vzt::LogicalDevice* logicalDevice, vzt::GraphicPipeline* graphicPipeline,
                     const vzt::Model& model, uint32_t imageCount);

        RenderObject(const RenderObject&) = delete;
        RenderObject& operator=(const RenderObject&) = delete;

        RenderObject(RenderObject&& other) noexcept;
        RenderObject& operator=(RenderObject&& other) noexcept;

        void Render(VkCommandBuffer commandBuffer, vzt::GraphicPipeline* graphicPipeline, uint32_t imageCount);
        void UpdateDescriptorSet(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer);
        void UpdateUniform(const vzt::MaterialInfo& materialInfo);
        void UpdatePushConstants(const vzt::Transforms& objectData);

        ~RenderObject();

    private:
        uint32_t m_imageCount{};
        LogicalDevice* m_logicalDevice = nullptr;

        std::unique_ptr<VertexBuffer> m_vertexBuffer;
        std::unique_ptr<IndexBuffer> m_indexBuffer;
        std::unordered_map<uint32_t, ImageHandler> m_textureHandlers;
        std::unordered_map<uint32_t, uint32_t> m_uniformRanges;

        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;
        std::vector<vzt::Buffer<vzt::MaterialInfo>> m_uniformBuffers;
        vzt::Transforms m_currentPushConstants{};
    };
}

#endif //VAZTERAN_RENDEROBJECT_HPP
