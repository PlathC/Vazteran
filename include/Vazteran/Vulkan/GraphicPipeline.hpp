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
        Shader vertexShader;
        Shader fragmentShader;
        VkExtent2D swapChainExtent;
        VkFormat swapChainImageFormat;
        DrawType drawType = DrawType::Fill;
    };

    class LogicalDevice;

    class GraphicPipeline {
    public:
        GraphicPipeline(LogicalDevice* logicalDevice, const PipelineSettings& settings);

        VkRenderPass RenderPass() const { return m_renderPass; }
        VkPipeline VkHandle() const { return m_vkHandle; }

        ~GraphicPipeline();

    private:
        LogicalDevice* m_logicalDevice;
        VkRenderPass m_renderPass;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_vkHandle;
    };
}

#endif //VAZTERAN_GRAPHICPIPELINE_HPP
