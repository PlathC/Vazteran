
#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/UiRenderer.hpp"
#include "Vazteran/Vulkan/VkRenderer.hpp"

namespace vzt {
    Renderer::Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<int> size, vzt::Camera camera,
                       std::vector<vzt::Model*> models):
            m_surface(surface), m_camera(camera),
            m_physicalDevice(std::make_unique<vzt::PhysicalDevice>(instance, m_surface)),
            m_logicalDevice(std::make_unique<vzt::LogicalDevice>(instance, m_physicalDevice.get(), m_surface)) {
        std::vector<std::unique_ptr<vzt::GraphicPipeline>> graphicPipelines;

        // Default graphic pipeline
        auto phongPipeline = std::make_unique<vzt::GraphicPipeline>(m_logicalDevice.get(), [&](VkCommandBuffer commandBuffer, const vzt::GraphicPipeline* graphicPipeline, uint32_t imageCount) {
            for (auto& object : m_objects) {
                object->Render(commandBuffer, graphicPipeline, imageCount);
            }
        });

        auto vtxShader  = vzt::Shader("./shaders/shader.vert.spv", ShaderStage::VertexShader);
        auto fragShader = vzt::Shader("./shaders/shader.frag.spv", ShaderStage::FragmentShader);
        
        vtxShader.AddPushConstant(sizeof(vzt::Transforms));
        phongPipeline->SetShader(vtxShader);

        fragShader.SetUniformDescriptorSet(0, sizeof(vzt::MaterialInfo));
        fragShader.SetSamplerDescriptorSet(1);
        fragShader.SetSamplerDescriptorSet(2);
        fragShader.SetSamplerDescriptorSet(3);

        phongPipeline->SetShader(fragShader);

        graphicPipelines.emplace_back(std::move(phongPipeline));

        /*auto uiPipeline = std::make_unique<vzt::GraphicPipeline>(m_logicalDevice.get(), [&](VkCommandBuffer commandBuffer, const vzt::GraphicPipeline* graphicPipeline, uint32_t imageCount) {
            m_uiRenderer->Update();
        });

        uiPipeline->Create();
        graphicPipelines.emplace_back(std::move(uiPipeline));

        m_uiRenderer = std::make_unique<vzt::UiRenderer>(window, instance, m_logicalDevice.get(), uiPipeline->RenderPass());
        */
        m_swapChain = std::make_unique<vzt::SwapChain>(m_logicalDevice.get(), std::move(graphicPipelines), surface, size);
        for(auto& model: models) {
            auto renderObject = std::make_unique<vzt::RenderObject>(m_logicalDevice.get(), m_swapChain->Pipeline(), model, m_swapChain->ImageCount());
            m_objects.emplace_back(std::move(renderObject));
        }
    }

    void Renderer::Draw() {
        for (auto& object: m_objects) {
            auto modelMatrix = object->Model()->ModelMatrix();
            auto viewMatrix = m_camera.View();
            auto projectionMatrix = m_camera.Projection();
            vzt::Transforms transforms {
                    modelMatrix,
                    viewMatrix,
                    projectionMatrix,
                    m_camera.position
            };

            transforms.projection[1][1] *= -1;
            transforms.viewPosition = m_camera.position;

            object->UpdatePushConstants(transforms);
        }

        // m_uiRenderer->Update();
        if(m_swapChain->DrawFrame()) {
            vkDeviceWaitIdle(m_logicalDevice->VkHandle());

            auto size = m_swapChain->FrameBufferSize();
            m_camera.aspectRatio = static_cast<float>(size.width) / static_cast<float>(size.height);
        }
    }

    RenderPass* Renderer::FinalPass() const {
        return m_swapChain->Pipeline()->RenderPass(); 
    }

    void Renderer::FrameBufferResized(vzt::Size2D<int> newSize) {
        m_swapChain->FrameBufferResized(newSize);
        m_swapChain->Recreate(m_surface);
    }
}