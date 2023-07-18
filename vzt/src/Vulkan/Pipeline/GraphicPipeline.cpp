#include "vzt/Vulkan/Pipeline/GraphicPipeline.hpp"

#include <cassert>

#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/RenderPass.hpp"

namespace vzt
{
    GraphicPipeline::GraphicPipeline(View<Device> device) : m_device(device) {}

    GraphicPipeline::GraphicPipeline(GraphicPipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_attachments, other.m_attachments);
        std::swap(m_program, other.m_program);
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_vertexDescription, other.m_vertexDescription);
        std::swap(m_viewport, other.m_viewport);
        std::swap(m_rasterization, other.m_rasterization);
        std::swap(m_multiSample, other.m_multiSample);
        std::swap(m_depthStencil, other.m_depthStencil);
    }

    GraphicPipeline& GraphicPipeline::operator=(GraphicPipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_attachments, other.m_attachments);
        std::swap(m_program, other.m_program);
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_vertexDescription, other.m_vertexDescription);
        std::swap(m_viewport, other.m_viewport);
        std::swap(m_rasterization, other.m_rasterization);
        std::swap(m_multiSample, other.m_multiSample);
        std::swap(m_depthStencil, other.m_depthStencil);

        return *this;
    }

    GraphicPipeline::~GraphicPipeline()
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

    std::tuple<VkViewport, VkRect2D>               toVulkan(const Viewport& viewport);
    VkPipelineRasterizationStateCreateInfo         toVulkan(const Rasterization& config);
    VkPipelineMultisampleStateCreateInfo           toVulkan(const MultiSampling& config);
    VkPipelineDepthStencilStateCreateInfo          toVulkan(const DepthStencil& config);
    std::vector<VkVertexInputBindingDescription>   toVulkan(std::vector<VertexBinding> bindings);
    std::vector<VkVertexInputAttributeDescription> toVulkan(std::vector<VertexAttribute> attributes);

    void GraphicPipeline::compile(View<RenderPass> renderPass)
    {
        assert(m_program && "You must provide a program before compiling pipeline.");

        if (m_compiled)
            cleanup();

        m_cachedRenderPass = renderPass;

        VkDescriptorSetLayout descriptorSetLayout = m_descriptorLayout.getHandle();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreatePipelineLayout(m_device->getHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
                "failed to create pipeline layout!");

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        std::vector<VkVertexInputBindingDescription>   vertexBindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions{};
        if (m_vertexDescription)
        {
            vertexBindingDescriptions   = toVulkan(m_vertexDescription->bindings);
            vertexAttributeDescriptions = toVulkan(m_vertexDescription->attributes);

            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
            vertexInputInfo.pVertexAttributeDescriptions    = vertexAttributeDescriptions.data();

            vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions    = vertexBindingDescriptions.data();
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = toVulkan(m_primitiveTopology);
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

        if (m_attachments.empty())
        {
            const auto& colorAttachments = renderPass->getColorAttachments();
            for (const auto& _ : colorAttachments)
                m_attachments.emplace_back(ColorMask::RGBA);
        }

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
        colorBlendAttachments.reserve(m_attachments.size());
        for (const auto& m_attachment : m_attachments)
        {
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = toVulkan(m_attachment);
            colorBlendAttachment.blendEnable    = VK_FALSE;
            colorBlendAttachments.emplace_back(colorBlendAttachment);
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable   = VK_FALSE;
        colorBlending.logicOp         = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments    = colorBlendAttachments.data();

        const auto& shaderModules = m_program->getModules();

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
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

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages             = shaderStages.data();
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

        vkCheck(table.vkCreateGraphicsPipelines(m_device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                                &m_handle),
                "Failed to create graphics pipeline.");

        m_compiled = true;
    }

    void GraphicPipeline::resize(Viewport viewport)
    {
        assert(m_cachedRenderPass && "Resize must be used after compilation.");

        m_viewport = std::move(viewport);
        compile(m_cachedRenderPass);
    }

    void GraphicPipeline::cleanup()
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
