#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/RenderTarget.hpp"

namespace vzt {
    RenderTarget::RenderTarget(vzt::LogicalDevice* logicalDevice, vzt::GraphicPipeline* graphicPipeline,
                                   const vzt::Model& model, uint32_t imageCount):
            m_imageCount(imageCount), m_logicalDevice(logicalDevice), m_graphicPipeline(graphicPipeline) {

        for (auto& shader : m_graphicPipeline->Shaders()) {
            auto samplerDescriptorSets = shader.SamplerDescriptorSets();

            if (samplerDescriptorSets.size() == 3) {
                m_textureHandlers.insert(std::make_pair<uint32_t, vzt::ImageHandler>(
                        std::move(samplerDescriptorSets[0].binding), {
                                vzt::ImageView(m_logicalDevice, model.CMat().ambientMap, VK_FORMAT_R8G8B8A8_SRGB),
                                vzt::Sampler(m_logicalDevice)
                        })
                );

                m_textureHandlers.insert(std::make_pair<uint32_t, vzt::ImageHandler>(
                        std::move(samplerDescriptorSets[1].binding), {
                                vzt::ImageView(m_logicalDevice, model.CMat().diffuseMap, VK_FORMAT_R8G8B8A8_SRGB),
                                vzt::Sampler(m_logicalDevice)
                        })
                );

                m_textureHandlers.insert(std::make_pair<uint32_t, vzt::ImageHandler>(
                        std::move(samplerDescriptorSets[2].binding), {
                                vzt::ImageView(m_logicalDevice, model.CMat().specularMap, VK_FORMAT_R8G8B8A8_SRGB),
                                vzt::Sampler(m_logicalDevice)
                        })
                );
            }

            auto uniformDescriptorSets = shader.UniformDescriptorSets();
            for(auto& uniform : uniformDescriptorSets) {
                m_uniformRanges[uniform.binding] = uniform.size;
            }
        }


        auto descriptorTypes = m_graphicPipeline->DescriptorTypes();
        auto poolSizes = std::vector<VkDescriptorPoolSize>(descriptorTypes.size());

        for (std::size_t i = 0; i < descriptorTypes.size(); i++) {
            poolSizes[i].type = descriptorTypes[i];
            poolSizes[i].descriptorCount = static_cast<uint32_t>(m_imageCount);
        }

        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = static_cast<uint32_t>(m_imageCount);

        if (vkCreateDescriptorPool(logicalDevice->VkHandle(), &descriptorPoolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        auto layouts = std::vector<VkDescriptorSetLayout>(m_imageCount, m_graphicPipeline->DescriptorSetLayout());
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = m_descriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(m_imageCount);
        descriptorSetAllocInfo.pSetLayouts = layouts.data();

        m_descriptorSets = std::vector<VkDescriptorSet>(m_imageCount);
        if (vkAllocateDescriptorSets(logicalDevice->VkHandle(), &descriptorSetAllocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        m_vertexBuffer = std::make_unique<vzt::VertexBuffer>(logicalDevice, model.Vertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_indexBuffer  = std::make_unique<vzt::IndexBuffer>(logicalDevice, model.Indices(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        for (std::size_t i = 0; i < m_descriptorSets.size(); i++) {
            m_uniformBuffers.emplace_back(
                    m_logicalDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            UpdateDescriptorSet(m_descriptorSets[i], m_uniformBuffers[i].VkHandle());
        }
    }

    RenderTarget::RenderTarget(RenderTarget&& other) noexcept {
        std::swap(m_imageCount, other.m_imageCount);
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_vertexBuffer, other.m_vertexBuffer);
        std::swap(m_indexBuffer, other.m_indexBuffer);
        std::swap(m_textureHandlers, other.m_textureHandlers);
        std::swap(m_uniformRanges, other.m_uniformRanges);
        m_descriptorPool = std::exchange(other.m_descriptorPool, static_cast<decltype(m_descriptorPool)>(VK_NULL_HANDLE));        std::swap(m_descriptorSets, other.m_descriptorSets);
        std::swap(m_uniformBuffers, other.m_uniformBuffers);
        std::swap(m_graphicPipeline, other.m_graphicPipeline);
    }

    RenderTarget& RenderTarget::operator=(RenderTarget&& other)  noexcept {
        std::swap(m_imageCount, other.m_imageCount);
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_vertexBuffer, other.m_vertexBuffer);
        std::swap(m_indexBuffer, other.m_indexBuffer);
        std::swap(m_textureHandlers, other.m_textureHandlers);
        std::swap(m_uniformRanges, other.m_uniformRanges);
        std::swap(m_descriptorPool, other.m_descriptorPool);
        std::swap(m_descriptorSets, other.m_descriptorSets);
        std::swap(m_uniformBuffers, other.m_uniformBuffers);
        std::swap(m_graphicPipeline, other.m_graphicPipeline);

        return *this;
    }

    void RenderTarget::Render(VkCommandBuffer commandBuffer, uint32_t imageCount) {
        m_graphicPipeline->Bind(commandBuffer);
        VkBuffer vertexBuffers[] = { m_vertexBuffer->VkHandle() };
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->VkHandle(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline->Layout(), 0, 1, &m_descriptorSets[imageCount], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indexBuffer->Size()), 1, 0, 0, 0);
    }

    void RenderTarget::UpdateDescriptorSet(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer) {
        auto descriptorWrites = std::vector<VkWriteDescriptorSet>();
        auto descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(m_uniformRanges.size());
        std::size_t i = 0;
        for(const auto& uniformRanges: m_uniformRanges) {
            descriptorBufferInfo[i].buffer = uniformBuffer;
            descriptorBufferInfo[i].offset = 0;
            descriptorBufferInfo[i].range = uniformRanges.second;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = uniformRanges.first;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &descriptorBufferInfo[i];
            descriptorWrites.emplace_back(descriptorWrite);
            i++;
        }

        i = 0;
        auto descriptorImageInfo = std::vector<VkDescriptorImageInfo>(m_textureHandlers.size());
        for(const auto& textureHandler: m_textureHandlers) {
            descriptorImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo[i].imageView = textureHandler.second.imageView.VkHandle();
            descriptorImageInfo[i].sampler = textureHandler.second.sampler.VkHandle();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = textureHandler.first;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &descriptorImageInfo[i];
            descriptorWrites.emplace_back(descriptorWrite);
            i++;
        }

        vkUpdateDescriptorSets(m_logicalDevice->VkHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    //TODO: Use template and more optimized synchronization behaviour
    void RenderTarget::UpdateUniform(const vzt::ObjectData& objectData) {
        vkDeviceWaitIdle(m_logicalDevice->VkHandle());
        for (auto& uniformBuffer: m_uniformBuffers) {
            uniformBuffer.Update<vzt::ObjectData>({objectData});
        }
    }

    RenderTarget::~RenderTarget() {
        if (m_descriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(m_logicalDevice->VkHandle(), m_descriptorPool, nullptr);
    }
}