#include <utility>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/GpuObjects.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"

namespace vzt
{
	GraphicPipeline::GraphicPipeline(vzt::Program&&                             program,
	                                 std::optional<vzt::DescriptorLayout>       userDefinedDescriptorLayout,
	                                 std::optional<vzt::VertexInputDescription> vertexInputDescription)
	    : m_program(std::move(program)), m_userDefinedDescriptorLayout(std::move(userDefinedDescriptorLayout)),
	      m_vertexInputDescription(std::move(vertexInputDescription))
	{
	}

	void GraphicPipeline::create()
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		if (m_vertexInputDescription)
		{
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount =
			    static_cast<uint32_t>(m_vertexInputDescription->attribute.size());
			vertexInputInfo.pVertexBindingDescriptions   = &m_vertexInputDescription->binding;
			vertexInputInfo.pVertexAttributeDescriptions = m_vertexInputDescription->attribute.data();
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float>(m_contextSettings.targetSize.width);
		viewport.height   = static_cast<float>(m_contextSettings.targetSize.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = {m_contextSettings.targetSize.width, m_contextSettings.targetSize.height};

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports    = &viewport;
		viewportState.scissorCount  = 1;
		viewportState.pScissors     = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable        = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode             = static_cast<VkPolygonMode>(m_rasterOptions.drawType);
		rasterizer.lineWidth               = 1.0f;
		rasterizer.cullMode                = static_cast<VkCullModeFlags>(m_rasterOptions.cullMode);
		rasterizer.frontFace               = static_cast<VkFrontFace>(m_rasterOptions.frontFace);
		rasterizer.depthBiasEnable         = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp          = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor    = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable   = VK_FALSE;
		multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading      = 1.0f;     // Optional
		multisampling.pSampleMask           = nullptr;  // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable      = VK_FALSE; // Optional

		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		colorBlendAttachments.reserve(m_contextSettings.attachmentCount);
		for (std::size_t i = 0; i < m_contextSettings.attachmentCount; i++)
		{
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			// colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
			// colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			// colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
			// colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
			// colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			// colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;      // Optional

			colorBlendAttachments.emplace_back(colorBlendAttachment);
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable     = VK_FALSE;
		colorBlending.logicOp           = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount   = static_cast<uint32_t>(colorBlendAttachments.size());
		colorBlending.pAttachments      = colorBlendAttachments.data();
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable  = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;

		const auto& shaderStages = m_program.getPipelineStages();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());

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
		pipelineInfo.renderPass         = m_contextSettings.renderPassTemplate->vkHandle();
		pipelineInfo.subpass            = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex  = -1;             // Optional

		if (vkCreateGraphicsPipelines(m_contextSettings.device->vkHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
		                              &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}
	}

	void GraphicPipeline::configure(vzt::PipelineContextSettings settings)
	{
		cleanup();

		m_contextSettings = std::move(settings);

		if (!m_program.isCompiled())
			m_program.compile(m_contextSettings.device);

		std::vector<VkDescriptorSetLayout> descriptors;
		descriptors.reserve(m_contextSettings.engineDescriptors.size());
		for (const auto& descriptor : m_contextSettings.engineDescriptors)
		{
			descriptors.emplace_back(descriptor->vkHandle());
		}

		if (m_userDefinedDescriptorLayout)
		{
			m_userDefinedDescriptorLayout->configure(m_contextSettings.device);
			descriptors.emplace_back(m_userDefinedDescriptorLayout->vkHandle());
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptors.size());
		pipelineLayoutInfo.pSetLayouts    = descriptors.data();

		if (vkCreatePipelineLayout(m_contextSettings.device->vkHandle(), &pipelineLayoutInfo, nullptr,
		                           &m_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		create();
	}

	void GraphicPipeline::bind(VkCommandBuffer commandsBuffer) const
	{
		assert(m_vkHandle != VK_NULL_HANDLE && "The engine must call configure before binding this pipeline");
		vkCmdBindPipeline(commandsBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkHandle);
	}

	void GraphicPipeline::cleanup()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_contextSettings.device->vkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}

		if (m_pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(m_contextSettings.device->vkHandle(), m_pipelineLayout, nullptr);
			m_pipelineLayout = VK_NULL_HANDLE;
		}
	}

	GraphicPipeline::~GraphicPipeline() { cleanup(); }
} // namespace vzt
