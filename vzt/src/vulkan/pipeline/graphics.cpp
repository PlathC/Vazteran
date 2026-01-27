#include "vzt/vulkan/pipeline/graphics.hpp"

#include <cassert>

#include "vzt/vulkan/command.hpp"
#include "vzt/vulkan/device.hpp"

namespace vzt
{
    GraphicsPipeline::GraphicsPipeline(GraphicsPipelineBuilder builder)
        : Pipeline(builder.program->getModules()[0].getDevice()), m_builder(std::move(builder))
    {
        m_pushConstants.insert(m_pushConstants.end(), m_builder.pushConstants.begin(), m_builder.pushConstants.end());
        compile();
    }

    GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept : Pipeline(std::move(other))
    {
        std::swap(m_builder, other.m_builder);
    }

    GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept
    {
        std::swap(m_builder, other.m_builder);

        Pipeline::operator=(std::move(other));
        return *this;
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        if (m_handle != VK_NULL_HANDLE)
        {
            const VolkDeviceTable& table = m_device->getFunctionTable();
            table.vkDestroyPipeline(m_device->getHandle(), m_handle, nullptr);
        }

        if (m_pipelineLayout != VK_NULL_HANDLE)
        {
            const VolkDeviceTable& table = m_device->getFunctionTable();
            table.vkDestroyPipelineLayout(m_device->getHandle(), m_pipelineLayout, nullptr);
        }
    }

    VkViewport                                     toVulkan(const Viewport& viewport);
    VkPipelineRasterizationStateCreateInfo         toVulkan(const Rasterization& config);
    VkPipelineMultisampleStateCreateInfo           toVulkan(const MultiSampling& config);
    VkPipelineDepthStencilStateCreateInfo          toVulkan(const DepthStencil& config);
    std::vector<VkVertexInputBindingDescription>   toVulkan(std::vector<VertexBinding> bindings);
    std::vector<VkVertexInputAttributeDescription> toVulkan(std::vector<VertexAttribute> attributes);

    void GraphicsPipeline::compile()
    {
        // Create descriptor layout
        m_descriptorLayout = DescriptorLayout(m_device);
        for (const auto& module : m_builder.program->getModules())
        {
            for (const auto [id, type] : module.getShader().bindings)
                m_descriptorLayout.addBinding(id, type);
        }

        m_descriptorLayout.compile();
        VkDescriptorSetLayout      descriptorSetLayout = m_descriptorLayout.getHandle();
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;

        auto pushConstants = std::vector<VkPushConstantRange>();
        if (!m_pushConstants.empty())
        {
            pushConstants.reserve(m_pushConstants.size());
            for (const PushConstant& pushConstant : m_pushConstants)
            {
                pushConstants.emplace_back(VkPushConstantRange{
                    .stageFlags = static_cast<VkShaderStageFlags>(pushConstant.stages),
                    .offset     = pushConstant.offset,
                    .size       = pushConstant.size,
                });
            }
            pipelineLayoutInfo.pPushConstantRanges    = pushConstants.data();
            pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        }

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreatePipelineLayout(m_device->getHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
                "failed to create pipeline layout!");

        // Create pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = m_pipelineLayout;

        // Rasterization
        const auto rasterizer            = toVulkan(m_builder.rasterization);
        pipelineInfo.pRasterizationState = &rasterizer;

        // Multisampling
        const auto multisampling       = toVulkan(m_builder.multiSampling);
        pipelineInfo.pMultisampleState = &multisampling;

        // Depth stencil
        const auto depthStencil         = toVulkan(m_builder.depthStencil);
        pipelineInfo.pDepthStencilState = &depthStencil;

        // vertex input
        VkPipelineVertexInputStateCreateInfo           vertexInputInfo{};
        std::vector<VkVertexInputBindingDescription>   vertexBindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions{};
        {
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            if (m_builder.inputDescription)
            {
                vertexBindingDescriptions   = toVulkan(m_builder.inputDescription->bindings);
                vertexAttributeDescriptions = toVulkan(m_builder.inputDescription->attributes);

                vertexInputInfo.vertexAttributeDescriptionCount =
                    static_cast<uint32_t>(vertexAttributeDescriptions.size());
                vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

                vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
                vertexInputInfo.pVertexBindingDescriptions    = vertexBindingDescriptions.data();
            }

            pipelineInfo.pVertexInputState = &vertexInputInfo;
        }

        // input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        {
            inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology               = toVulkan(m_builder.primitiveTopology);
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            pipelineInfo.pInputAssemblyState = &inputAssembly;
        }

        // viewport
        VkPipelineViewportStateCreateInfo viewportState{};
        {
            viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports    = nullptr;
            viewportState.scissorCount  = 1;
            viewportState.pScissors     = nullptr;

            pipelineInfo.pViewportState = &viewportState;
        }

        // viewport
        const auto&                                  shaderModules = m_builder.program->getModules();
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        {
            shaderStages.reserve(shaderModules.size());
            for (const auto& shaderModule : shaderModules)
            {
                const auto& shader = shaderModule.getShader();

                VkPipelineShaderStageCreateInfo createInfo{};
                createInfo.module = shaderModule.getHandle();
                createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                createInfo.stage  = toVulkan(shader.stage);
                createInfo.pName  = "main";

                shaderStages.emplace_back(createInfo);
            }
            pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
            pipelineInfo.pStages    = shaderStages.data();
        }

        VkPipelineColorBlendStateCreateInfo              colorBlending{};
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
        {
            colorBlendAttachments.reserve(m_builder.colors.size());
            for (uint32_t a = 0; a < m_builder.colors.size(); a++)
            {
                const ColorBlend& blend = m_builder.colors[a].blend;

                VkPipelineColorBlendAttachmentState colorBlendAttachment{};
                colorBlendAttachment.blendEnable         = blend.blendEnable;
                colorBlendAttachment.srcColorBlendFactor = toVulkan(blend.srcColorBlendFactor);
                colorBlendAttachment.dstColorBlendFactor = toVulkan(blend.dstColorBlendFactor);
                colorBlendAttachment.colorBlendOp        = toVulkan(blend.colorBlendOp);
                colorBlendAttachment.srcAlphaBlendFactor = toVulkan(blend.srcAlphaBlendFactor);
                colorBlendAttachment.dstAlphaBlendFactor = toVulkan(blend.dstAlphaBlendFactor);
                colorBlendAttachment.alphaBlendOp        = toVulkan(blend.alphaBlendOp);
                colorBlendAttachment.colorWriteMask      = toVulkan(blend.colorWriteMask);

                colorBlendAttachments.emplace_back(colorBlendAttachment);
            }

            colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable   = VK_FALSE;
            colorBlending.logicOp         = VK_LOGIC_OP_COPY; // Optional
            colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
            colorBlending.pAttachments    = colorBlendAttachments.data();

            pipelineInfo.pColorBlendState = &colorBlending;
        }

        // dynamic state
        VkPipelineDynamicStateCreateInfo dynamicState{};
        constexpr std::array             dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        {
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

            dynamicState.dynamicStateCount = dynamicStates.size();
            dynamicState.pDynamicStates    = dynamicStates.data();

            pipelineInfo.pDynamicState = &dynamicState;
        }

        std::vector<VkFormat>            colorFormats;
        VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
        {
            colorFormats.reserve(m_builder.colors.size());
            for (uint32_t a = 0; a < m_builder.colors.size(); a++)
                colorFormats.emplace_back(toVulkan(m_builder.colors[a].format));

            pipelineRenderingCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
            pipelineRenderingCreateInfo.colorAttachmentCount    = static_cast<uint32_t>(colorFormats.size());
            pipelineRenderingCreateInfo.pColorAttachmentFormats = colorFormats.data();
            pipelineRenderingCreateInfo.depthAttachmentFormat =
                m_builder.depth ? toVulkan(*m_builder.depth) : VK_FORMAT_UNDEFINED;
            pipelineRenderingCreateInfo.stencilAttachmentFormat =
                m_builder.depth ? toVulkan(*m_builder.depth) : VK_FORMAT_UNDEFINED;

            pipelineInfo.pNext = &pipelineRenderingCreateInfo;
        }

        pipelineInfo.pTessellationState = nullptr; //

        // Unused (dynamic rendering)
        pipelineInfo.renderPass         = nullptr;
        pipelineInfo.subpass            = 0;
        pipelineInfo.basePipelineHandle = nullptr;
        pipelineInfo.basePipelineIndex  = 0;

        vkCheck(table.vkCreateGraphicsPipelines(m_device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                                &m_handle),
                "Failed to create graphics pipeline.");
    }

    VkViewport toVulkan(const Viewport& viewport)
    {
        VkViewport result;
        result.x        = static_cast<float>(viewport.upperLeftCorner.x);
        result.y        = static_cast<float>(viewport.upperLeftCorner.y);
        result.width    = static_cast<float>(viewport.size.x);
        result.height   = static_cast<float>(viewport.size.y);
        result.minDepth = viewport.minDepth;
        result.maxDepth = viewport.maxDepth;

        return result;
    }

    VkPipelineRasterizationStateCreateInfo toVulkan(const Rasterization& config)
    {
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = config.depthClamp;
        rasterizer.rasterizerDiscardEnable = config.discardEnable;
        rasterizer.polygonMode             = toVulkan(config.polygonMode);
        rasterizer.lineWidth               = config.lineWidth;
        rasterizer.cullMode                = toVulkan(config.cullMode);
        rasterizer.frontFace               = toVulkan(config.frontFace);

        rasterizer.depthBiasEnable = VK_FALSE;
        // rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        // rasterizer.depthBiasClamp          = 0.0f; // Optional
        // rasterizer.depthBiasSlopeFactor    = 0.0f; // Optional
        return rasterizer;
    }

    VkPipelineMultisampleStateCreateInfo toVulkan(const MultiSampling& config)
    {
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable   = config.enable;
        multisampling.rasterizationSamples  = toVulkan(config.sampleCount);
        multisampling.minSampleShading      = 1.0f;     // Optional
        multisampling.pSampleMask           = nullptr;  // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable      = VK_FALSE; // Optional

        return multisampling;
    }

    VkPipelineDepthStencilStateCreateInfo toVulkan(const DepthStencil& config)
    {
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable  = config.enable;
        depthStencil.depthWriteEnable = config.depthWriteEnable;
        depthStencil.depthCompareOp   = toVulkan(config.compareOp);

        return depthStencil;
    }

    std::vector<VkVertexInputBindingDescription> toVulkan(std::vector<VertexBinding> bindings)
    {
        std::vector<VkVertexInputBindingDescription> descriptions{};
        descriptions.reserve(bindings.size());

        for (const auto& binding : bindings)
        {
            VkVertexInputBindingDescription description;
            description.inputRate = toVulkan(binding.inputRate);
            description.stride    = binding.stride;
            description.binding   = binding.binding;
            descriptions.emplace_back(description);
        }

        return descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> toVulkan(std::vector<VertexAttribute> attributes)
    {
        std::vector<VkVertexInputAttributeDescription> descriptions{};
        descriptions.reserve(attributes.size());

        for (const auto& attribute : attributes)
        {
            VkVertexInputAttributeDescription description;
            description.format   = toVulkan(attribute.dataFormat);
            description.offset   = attribute.offset;
            description.binding  = attribute.binding;
            description.location = attribute.location;
            descriptions.emplace_back(description);
        }

        return descriptions;
    }

} // namespace vzt
