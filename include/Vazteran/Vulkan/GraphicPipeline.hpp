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

        GraphicPipeline(const GraphicPipeline&) = delete;
        GraphicPipeline& operator=(const GraphicPipeline&) = delete;

        GraphicPipeline(GraphicPipeline&& other) noexcept;
        GraphicPipeline& operator=(GraphicPipeline&& other) noexcept;

        void Bind(VkCommandBuffer commandsBuffer) const;
        VkDescriptorSetLayout DescriptorSetLayout() const { return m_descriptorSetLayout; }
        RenderPass* RenderPass() const { return m_renderPass.get(); }
        VkPipelineLayout Layout() const { return m_pipelineLayout; }
        VkPipeline VkHandle() const { return m_vkHandle; }
        void UpdateDescriptorSet(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer) const;
        std::vector<VkDescriptorType> DescriptorTypes() const;
        std::unordered_set<vzt::Shader, vzt::ShaderHash> Shaders() { return m_shaders; };

        ~GraphicPipeline();

    private:
        vzt::LogicalDevice* m_logicalDevice;

        VkPipeline m_vkHandle{};
        VkPipelineLayout m_pipelineLayout;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::unique_ptr<vzt::RenderPass> m_renderPass;

        std::unordered_set<vzt::Shader, vzt::ShaderHash> m_shaders;
    };
}

#endif //VAZTERAN_GRAPHICPIPELINE_HPP
