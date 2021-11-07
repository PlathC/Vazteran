#ifndef VAZTERAN_GRAPHICPIPELINE_HPP
#define VAZTERAN_GRAPHICPIPELINE_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "Vazteran/Vulkan/Shader.hpp"

namespace vzt {

    class GraphicPipeline;
    class LogicalDevice;
    class ImageView;
    class RenderPass;
    class Sampler;

    enum class DrawType {
        Fill = VK_POLYGON_MODE_FILL,
        Line = VK_POLYGON_MODE_LINE,
        Point = VK_POLYGON_MODE_POINT
    };

    struct PipelineSettings {
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        vzt::DrawType drawType = DrawType::Fill;
    };

    using RenderFunction = std::function<void(VkCommandBuffer, const vzt::GraphicPipeline*, uint32_t)>;

    class GraphicPipeline {
    public:
        GraphicPipeline(vzt::LogicalDevice* logicalDevice, RenderFunction renderFunction = {});

        GraphicPipeline(const GraphicPipeline&) = delete;
        GraphicPipeline& operator=(const GraphicPipeline&) = delete;

        GraphicPipeline(GraphicPipeline&& other) noexcept;
        GraphicPipeline& operator=(GraphicPipeline&& other) noexcept;

        void Bind(VkCommandBuffer commandsBuffer) const;
        void Create();
        void Cleanup();
        void Configure(vzt::PipelineSettings settings);
        void Render(VkCommandBuffer commandBuffer, uint32_t bufferNumber) const;
        void SetRenderFunction(vzt::RenderFunction renderFunction);
        void SetShader(Shader shader);

        VkDescriptorSetLayout DescriptorSetLayout() const { return m_descriptorSetLayout; }
        RenderPass* RenderPass() const { return m_renderPass.get(); }
        VkPipelineLayout Layout() const { return m_pipelineLayout; }
        VkPipeline VkHandle() const { return m_vkHandle; }
        std::vector<VkDescriptorType> DescriptorTypes() const;
        std::unordered_set<vzt::Shader, vzt::ShaderHash> Shaders() { return m_shaders; };
        

        ~GraphicPipeline();

    private:
        vzt::LogicalDevice* m_logicalDevice;
        RenderFunction m_renderFunction;

        VkPipeline m_vkHandle = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        std::unique_ptr<vzt::RenderPass> m_renderPass;

        std::unordered_set<vzt::Shader, vzt::ShaderHash> m_shaders;
        vzt::PipelineSettings m_settings{};
    };
}

#endif //VAZTERAN_GRAPHICPIPELINE_HPP
