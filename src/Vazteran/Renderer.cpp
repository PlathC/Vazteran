#include "Vazteran/Renderer.hpp"
#include "Vazteran/Vulkan/GraphicPipeline.hpp"

namespace vzt {
    Renderer::Renderer(vzt::Instance* instance, VkSurfaceKHR surface, vzt::Size2D<int> size, vzt::Camera camera,
                       std::vector<vzt::Model*> models):
            m_surface(surface), m_camera(camera),
            m_physicalDevice(std::make_unique<vzt::PhysicalDevice>(instance, m_surface)),
            m_logicalDevice(std::make_unique<vzt::LogicalDevice>(instance, m_physicalDevice.get(), m_surface)),
            m_commandPool(m_logicalDevice.get(), MaxFramesInFlight)
    {
        m_swapChain = std::make_unique<vzt::SwapChain>(
            m_logicalDevice.get(), surface, size, &m_commandPool, MaxFramesInFlight
        );

        m_commandPool.SetRenderFunction([&](VkCommandBuffer commandBuffer, vzt::GraphicPipeline* graphicPipeline, uint32_t imageCount) {
            for (auto& object: m_objects){
                object->Render(commandBuffer, graphicPipeline, imageCount);
            }
        });

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

        if(m_swapChain->DrawFrame()) {
            vkDeviceWaitIdle(m_logicalDevice->VkHandle());

            auto size = m_swapChain->FrameBufferSize();
            m_camera.aspectRatio = static_cast<float>(size.width) / static_cast<float>(size.height);
        }
    }

    void Renderer::FrameBufferResized(vzt::Size2D<int> newSize) {
        m_swapChain->FrameBufferResized(newSize);
        m_swapChain->Recreate(m_surface);
    }
}