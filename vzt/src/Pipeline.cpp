#include "vzt/Pipeline.hpp"

#include "vzt/Device.hpp"
#include "vzt/RenderPass.hpp"

namespace vzt
{
    Pipeline::Pipeline(View<Device> device) : m_device(device) {}

    Pipeline::Pipeline(Pipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_viewport, other.m_viewport);
        std::swap(m_rasterization, other.m_rasterization);
        std::swap(m_multiSample, other.m_multiSample);
        std::swap(m_depthStencil, other.m_depthStencil);
    }

    Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_viewport, other.m_viewport);
        std::swap(m_rasterization, other.m_rasterization);
        std::swap(m_multiSample, other.m_multiSample);
        std::swap(m_depthStencil, other.m_depthStencil);

        return *this;
    }

    Pipeline::~Pipeline()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroyPipeline(m_device->getHandle(), m_handle, nullptr);

        if (m_pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(m_device->getHandle(), m_pipelineLayout, nullptr);
    }

    std::tuple<VkViewport, VkRect2D>       toVulkan(const Viewport& viewport);
    VkPipelineRasterizationStateCreateInfo toVulkan(const Rasterization& config);
    VkPipelineMultisampleStateCreateInfo   toVulkan(const MultiSampling& config);
    VkPipelineDepthStencilStateCreateInfo  toVulkan(const DepthStencil& config);

    void Pipeline::compile(View<RenderPass> renderPass)
    {
        if (m_compiled)
            cleanup();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        auto [viewport, scissor] = toVulkan(m_viewport);
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

        const auto rasterizer    = toVulkan(m_rasterization);
        const auto multisampling = toVulkan(m_multiSample);
        const auto depthStencil  = toVulkan(m_depthStencil);

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp       = VK_LOGIC_OP_COPY; // Optional
        // colorBlending.attachmentCount   = static_cast<uint32_t>(colorBlendAttachments.size());
        // colorBlending.pAttachments      = colorBlendAttachments.data();
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        // const auto& shaderStages = m_program.getPipelineStages();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        // pipelineInfo.pStages             = shaderStages.data();
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pDepthStencilState  = &depthStencil;
        pipelineInfo.pColorBlendState    = &colorBlending;

        pipelineInfo.pDynamicState      = nullptr; // Optional
        pipelineInfo.layout             = m_pipelineLayout;
        pipelineInfo.renderPass         = renderPass->getHandle();
        pipelineInfo.subpass            = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex  = -1;             // Optional

        vkCheck(vkCreateGraphicsPipelines(m_device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_handle),
                "Failed to create graphics pipeline.");

        m_compiled = true;
    }

    void Pipeline::cleanup()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroyPipeline(m_device->getHandle(), m_handle, nullptr);

        if (m_pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(m_device->getHandle(), m_pipelineLayout, nullptr);

        m_compiled = false;
    }

    std::tuple<VkViewport, VkRect2D> toVulkan(const Viewport& viewport)
    {
        VkViewport result;
        result.x        = static_cast<float>(viewport.upperLeftCorner.x);
        result.y        = static_cast<float>(viewport.upperLeftCorner.y);
        result.width    = static_cast<float>(viewport.size.width);
        result.height   = static_cast<float>(viewport.size.height);
        result.minDepth = viewport.minDepth;
        result.maxDepth = viewport.maxDepth;

        VkRect2D scissor{};
        scissor.extent = {viewport.size.width, viewport.size.height};
        if (viewport.scissor)
        {
            scissor.offset.x      = viewport.scissor->offset.x;
            scissor.offset.y      = viewport.scissor->offset.y;
            scissor.extent.width  = viewport.size.width;
            scissor.extent.height = viewport.size.height;
        }

        return {result, scissor};
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
} // namespace vzt
