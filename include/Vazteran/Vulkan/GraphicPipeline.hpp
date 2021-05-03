#ifndef VAZTERAN_GRAPHICPIPELINE_HPP
#define VAZTERAN_GRAPHICPIPELINE_HPP

#include <iostream>
#include <unordered_map>

#include "Vazteran/Vulkan/Shader.hpp"

namespace vzt {
    enum DrawType {
        Fill = VK_POLYGON_MODE_FILL,
        Line = VK_POLYGON_MODE_LINE,
        Point = VK_POLYGON_MODE_POINT
    };

    struct PipelineSettings {
        vzt::Shader vertexShader;
        vzt::Shader fragmentShader;
        VkExtent2D swapChainExtent;
        VkFormat swapChainImageFormat;
        vzt::DrawType drawType = DrawType::Fill;
    };

    class LogicalDevice;

    class GraphicPipeline {
    public:
        GraphicPipeline(vzt::LogicalDevice* logicalDevice, const vzt::PipelineSettings& settings);

        VkDescriptorSetLayout DescriptorSetLayout() const { return m_descriptorSetLayout; }
        VkRenderPass RenderPass() const { return m_renderPass; }
        VkPipelineLayout Layout() const { return m_pipelineLayout; }
        VkPipeline VkHandle() const { return m_vkHandle; }

        ~GraphicPipeline();

    private:
        void CreateRenderPass();

        vzt::LogicalDevice* m_logicalDevice;
        VkFormat m_colorImageFormat;
        VkRenderPass m_renderPass;
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_vkHandle;
    };
}

#endif //VAZTERAN_GRAPHICPIPELINE_HPP
