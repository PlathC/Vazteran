#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/GpuObjects.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/RenderPass.hpp"

namespace vzt {
    GraphicPipeline::GraphicPipeline(vzt::LogicalDevice* logicalDevice, vzt::PipelineSettings settings):
            m_logicalDevice(logicalDevice), m_renderPass(std::move(settings.renderPass)) {

        auto vertexShader = Shader("./shaders/shader.vert.spv", ShaderStage::VertexShader);
        vertexShader.SetUniformDescriptorSet(0, sizeof(vzt::Transforms));
        m_shaders.emplace(vertexShader);

        auto fragShader = Shader("./shaders/shader.frag.spv", ShaderStage::FragmentShader);
        fragShader.SetSamplerDescriptorSet(1, Image());
        fragShader.SetSamplerDescriptorSet(2, Image());
        fragShader.SetSamplerDescriptorSet(3, Image());
        m_shaders.emplace(fragShader);

        std::vector<std::unique_ptr<vzt::ShaderModule>> shaderModules{};
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        auto buildDescriptorSetLayoutBinding = [](vzt::ShaderStage stage, uint32_t binding, VkDescriptorType type) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            layoutBinding.descriptorCount = 1;
            layoutBinding.pImmutableSamplers = nullptr; // Optional
            layoutBinding.stageFlags = static_cast<VkShaderStageFlags>(stage);
            layoutBinding.descriptorType = type;
            return layoutBinding;
        };

        for (auto& shader : m_shaders) {
            for (const auto descriptor: shader.DescriptorTypes()) {
                bindings.emplace_back(buildDescriptorSetLayoutBinding(shader.Stage(), descriptor.first, descriptor.second));
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
        // std::swap(m_textureHandlers, other.m_textureHandlers);
        // std::swap(m_uniformRanges, other.m_uniformRanges);
    }

    GraphicPipeline& GraphicPipeline::operator=(GraphicPipeline&& other) noexcept {
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_vkHandle, other.m_vkHandle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_descriptorSetLayout, other.m_descriptorSetLayout);
        std::swap(m_renderPass, other.m_renderPass);
        // std::swap(m_textureHandlers, other.m_textureHandlers);
        // std::swap(m_uniformRanges, other.m_uniformRanges);

        return *this;
    }

    void GraphicPipeline::Bind(VkCommandBuffer commandsBuffer) const {
        vkCmdBindPipeline(commandsBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkHandle);
    }

    std::vector<VkDescriptorType> GraphicPipeline::DescriptorTypes() const {
        std::vector<VkDescriptorType> descriptorTypes;
        for (auto& shader : m_shaders) {
            auto types = shader.DescriptorTypes();
            for (const auto& type : types)
                descriptorTypes.emplace_back(type.second);
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
