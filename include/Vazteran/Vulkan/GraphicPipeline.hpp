#ifndef VAZTERAN_GRAPHICPIPELINE_HPP
#define VAZTERAN_GRAPHICPIPELINE_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "Vazteran/Vulkan/Shader.hpp"

namespace vzt {
    enum DrawType {
        Fill = VK_POLYGON_MODE_FILL,
        Line = VK_POLYGON_MODE_LINE,
        Point = VK_POLYGON_MODE_POINT
    };

    class RenderPass;
    struct PipelineSettings {
        std::unordered_set<vzt::Shader, vzt::ShaderHash> shaders;
        std::unique_ptr<vzt::RenderPass> renderPass;
        VkExtent2D swapChainExtent;
        vzt::DrawType drawType = DrawType::Fill;
    };

    class LogicalDevice;
    class ImageView;
    class Sampler;

    class GraphicPipeline {
    public:
        GraphicPipeline(vzt::LogicalDevice* logicalDevice, vzt::PipelineSettings settings);

        VkDescriptorSetLayout DescriptorSetLayout() const { return m_descriptorSetLayout; }
        RenderPass* RenderPass() const { return m_renderPass.get(); }
        VkPipelineLayout Layout() const { return m_pipelineLayout; }
        VkPipeline VkHandle() const { return m_vkHandle; }
        void UpdateDescriptorSet(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer) const;
        std::vector<VkDescriptorType> DescriptorTypes() const;

        ~GraphicPipeline();

    private:
        std::unique_ptr<vzt::RenderPass> m_renderPass;
        vzt::LogicalDevice* m_logicalDevice;

        std::unordered_map<uint32_t, TextureHandler> m_textureHandlers;
        std::unordered_map<uint32_t, uint32_t> m_uniformRanges;

        VkDescriptorSetLayout m_descriptorSetLayout;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_vkHandle{};
    };
}

#endif //VAZTERAN_GRAPHICPIPELINE_HPP
