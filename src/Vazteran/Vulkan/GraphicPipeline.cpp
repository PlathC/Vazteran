#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/GpuObject.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/RenderPass.hpp"
#include "Vazteran/Vulkan/Texture.hpp"

namespace vzt {

    GraphicPipeline::GraphicPipeline(vzt::LogicalDevice* logicalDevice, vzt::PipelineSettings settings):
            m_logicalDevice(logicalDevice), m_renderPass(std::move(settings.renderPass)) {

        std::vector<std::unique_ptr<vzt::ShaderModule>> shaderModules{};
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        auto buildDescriptorSetLayoutBinding = [](const DescriptorSet* descriptorSet, vzt::ShaderStage stage) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = descriptorSet->binding;
            layoutBinding.descriptorCount = 1;
            layoutBinding.pImmutableSamplers = nullptr; // Optional
            layoutBinding.stageFlags = stage;
            return layoutBinding;
        };

        for (auto& shader : settings.shaders) {
            auto samplerDescriptorSets = shader.SamplerDescriptorSets();
            for(auto& sampler : samplerDescriptorSets) {
                VkDescriptorSetLayoutBinding layoutBinding = buildDescriptorSetLayoutBinding(&sampler, shader.Stage());
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                m_textureHandlers[layoutBinding.binding] = {
                    std::make_unique<ImageView>(m_logicalDevice, sampler.image),
                    std::make_unique<Sampler>(m_logicalDevice)
                };
                bindings.emplace_back(layoutBinding);
            }

            auto uniformDescriptorSets = shader.UniformDescriptorSets();
            for(auto& uniform : uniformDescriptorSets) {
                VkDescriptorSetLayoutBinding layoutBinding = buildDescriptorSetLayoutBinding(&uniform, shader.Stage());
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                bindings.emplace_back(layoutBinding);
                m_uniformRanges[layoutBinding.binding] = uniform.size;
            }

            auto module = std::make_unique<ShaderModule>(m_logicalDevice, shader.ShaderModuleCreateInfo());
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage = static_cast<VkShaderStageFlagBits>(shader.Stage());
            shaderStageCreateInfo.module = module->VkHandle();
            shaderStageCreateInfo.pName = "main";

            shaderModules.emplace_back(std::move(module));
            shaderStages.emplace_back(shaderStageCreateInfo);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_logicalDevice->VkHandle(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(settings.swapChainExtent.width);
        viewport.height = static_cast<float>(settings.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = settings.swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = static_cast<VkPolygonMode>(settings.drawType);
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                              | VK_COLOR_COMPONENT_G_BIT
                                              | VK_COLOR_COMPONENT_B_BIT
                                              | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

        if (vkCreatePipelineLayout(m_logicalDevice->VkHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;

        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass->VkHandle();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(m_logicalDevice->VkHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkHandle)
            != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
    }

    GraphicPipeline::GraphicPipeline(GraphicPipeline&& other) noexcept {
        m_logicalDevice = std::exchange(other.m_logicalDevice, nullptr);
        m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
        m_pipelineLayout = std::exchange(other.m_pipelineLayout, static_cast<decltype(m_pipelineLayout)>(VK_NULL_HANDLE));
        m_descriptorSetLayout = std::exchange(other.m_descriptorSetLayout, static_cast<decltype(m_descriptorSetLayout)>(VK_NULL_HANDLE));

        std::swap(m_renderPass, other.m_renderPass);
        std::swap(m_textureHandlers, other.m_textureHandlers);
        std::swap(m_uniformRanges, other.m_uniformRanges);
    }

    GraphicPipeline& GraphicPipeline::operator=(GraphicPipeline&& other) noexcept {
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_vkHandle, other.m_vkHandle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_descriptorSetLayout, other.m_descriptorSetLayout);
        std::swap(m_renderPass, other.m_renderPass);
        std::swap(m_textureHandlers, other.m_textureHandlers);
        std::swap(m_uniformRanges, other.m_uniformRanges);

        return *this;
    }

    void GraphicPipeline::UpdateDescriptorSet(VkDescriptorSet descriptorSet, VkBuffer uniformBuffer) const {
        auto descriptorWrites = std::vector<VkWriteDescriptorSet>();
        auto descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(m_uniformRanges.size());
        std::size_t i = 0;
        for(const auto& uniformRanges : m_uniformRanges) {
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
        for(const auto& textureHandler : m_textureHandlers) {
            descriptorImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo[i].imageView = textureHandler.second.imageView->VkHandle();
            descriptorImageInfo[i].sampler = textureHandler.second.sampler->VkHandle();

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

    std::vector<VkDescriptorType> GraphicPipeline::DescriptorTypes() const {
        std::vector<VkDescriptorType> descriptorTypes;
        for(const auto& uniformRanges : m_uniformRanges) {
            descriptorTypes.emplace_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }

        for(const auto& textureHandler : m_textureHandlers) {
            descriptorTypes.emplace_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }
        return descriptorTypes;
    }

    GraphicPipeline::~GraphicPipeline() {
        if (m_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_logicalDevice->VkHandle(), m_descriptorSetLayout, nullptr);
        }

        if (m_vkHandle != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
        }

        if (m_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_logicalDevice->VkHandle(), m_pipelineLayout, nullptr);
        }
    }
}
