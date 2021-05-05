#include "Vazteran/Renderer.hpp"
#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    Renderer::Renderer(vzt::Instance* instance, VkSurfaceKHR surface, vzt::Size2D size):
            m_surface(surface) {
        m_physicalDevice = std::make_unique<vzt::PhysicalDevice>(instance, m_surface);
        m_logicalDevice  = std::make_unique<vzt::LogicalDevice>(instance, m_physicalDevice.get(), m_surface);

        m_model = std::make_unique<vzt::Model>("./samples/viking_room.obj");
        auto texture = Image("./samples/viking_room.png");
        m_model->Mat().ambient = texture;
        m_model->Mat().diffuse = texture;
        m_model->Mat().specular = glm::vec4(0., 0., 0., 0.);

        m_camera = Camera::FromModel(*m_model, size.width / static_cast<float>(size.height));


        // TODO: Integrate this in the Renderer class
        m_vertexBuffer = std::make_unique<VertexBuffer>(m_logicalDevice.get(), m_model->Vertices(),
                                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_indexBuffer  = std::make_unique<IndexBuffer>(m_logicalDevice.get(), m_model->Indices(),
                                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT);


        std::unordered_set<vzt::Shader, vzt::ShaderHash> shaders;
        shaders.emplace("./shaders/shader.vert.spv", ShaderStage::VertexShader,
                        std::vector<SamplerDescriptorSet>{},
                        std::vector<UniformDescriptorSet>{{0, sizeof(vzt::Transforms)}});
        shaders.emplace("./shaders/shader.frag.spv", ShaderStage::FragmentShader,
                        std::vector<SamplerDescriptorSet>{
                                {1, m_model->Mat().ambient},
                                {2, m_model->Mat().diffuse},
                                {3, m_model->Mat().specular},},
                        std::vector<UniformDescriptorSet>{ }
        );

        m_swapChain = std::make_unique<vzt::SwapChain>(
                m_logicalDevice.get(), surface, size.width, size.height,
                [&](VkCommandBuffer commandBuffer, VkDescriptorSet& descriptorSet, GraphicPipeline* graphicPipeline) {
                    VkBuffer vertexBuffers[] = { m_vertexBuffer->VkHandle() };
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->VkHandle(), 0, VK_INDEX_TYPE_UINT32);

                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline->Layout(), 0, 1, &descriptorSet, 0, nullptr);
                    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indexBuffer->Size()), 1, 0, 0, 0);
                },
                std::move(shaders)
        );
    }

    void Renderer::Draw() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        m_model->Rotation() = time * glm::radians(45.0f) * glm::vec3(0.0f, 0.0f, 1.0f);
        vzt::Transforms ubo{
                m_model->ModelMatrix(),
                m_camera.View(),
                m_camera.Projection(),
                m_camera.position
        };

        ubo.projection[1][1] *= -1;
        ubo.viewPosition = m_camera.position;

        if(m_swapChain->DrawFrame(ubo)) {
            vkDeviceWaitIdle(m_logicalDevice->VkHandle());
            auto size = m_swapChain->FrameBufferSize();
            m_camera.aspectRatio = static_cast<float>(size.width) / static_cast<float>(size.height);
        }
    }

    void Renderer::FrameBufferResized(vzt::Size2D newSize) {
        m_swapChain->FrameBufferResized(newSize);
        m_swapChain->Recreate(m_surface);
    }
}