#include "Vazteran/Renderer.hpp"
#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    Renderer::Renderer(vzt::Instance* instance, VkSurfaceKHR surface, vzt::Size2D<int> size, vzt::Camera camera,
                       std::vector<vzt::Model*> models):
            m_surface(surface), m_camera(camera) {

        m_physicalDevice = std::make_unique<vzt::PhysicalDevice>(instance, m_surface);
        m_logicalDevice  = std::make_unique<vzt::LogicalDevice>(instance, m_physicalDevice.get(), m_surface);

        m_swapChain = std::make_unique<vzt::SwapChain>(
            m_logicalDevice.get(), surface, size
        );

        for(auto& model: models) {
            m_targets.emplace_back(ModelRenderTarget{
                    model,
                    vzt::RenderTarget(m_logicalDevice.get(), m_swapChain->Pipeline(), *model, m_swapChain->ImageCount())
            });
        }

        m_swapChain->RecordCommandBuffers(
            [&](VkCommandBuffer commandBuffer, uint32_t imageCount) {
                for (auto& target: m_targets){
                    target.vkTarget.Render(commandBuffer, imageCount);
                }
            }
        );
    }

    void Renderer::Draw() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        for (auto& target: m_targets) {
            target.model->Rotation() = time * glm::radians(45.0f) * glm::vec3(0.0f, 0.0f, 1.0f);
            vzt::ObjectData ubo {
                    target.model->CMat().ambientColor,
                    target.model->CMat().diffuseColor,
                    target.model->CMat().specularColor,
                    target.model->ModelMatrix(),
                    m_camera.View(),
                    m_camera.Projection(),
                    m_camera.position,
                    target.model->CMat().shininess
            };

            ubo.projection[1][1] *= -1;
            ubo.viewPosition = m_camera.position;

            target.vkTarget.UpdateUniform(ubo);
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

        for(auto& target: m_targets) {
            target.vkTarget = vzt::RenderTarget(m_logicalDevice.get(), m_swapChain->Pipeline(), *target.model, m_swapChain->ImageCount());
        }

        m_swapChain->RecordCommandBuffers(
                [&](VkCommandBuffer commandBuffer, uint32_t imageCount) {
                    for (auto& target: m_targets){
                        target.vkTarget.Render(commandBuffer, imageCount);
                    }
                }
        );
    }
}