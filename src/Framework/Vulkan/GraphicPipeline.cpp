#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Framework/Vulkan/Device.hpp"
#include "Vazteran/Framework/Vulkan/GpuObjects.hpp"
#include "Vazteran/Framework/Vulkan/RenderPass.hpp"

namespace vzt
{
	GraphicPipeline::GraphicPipeline(vzt::Device *logicalDevice, RenderFunction renderFunction)
	    : m_logicalDevice(logicalDevice), m_renderFunction(renderFunction)
	{
	}

	void GraphicPipeline::Create()
	{
		std::vector<std::unique_ptr<vzt::ShaderModule>> shaderModules{};
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		auto buildDescriptorSetLayoutBinding = [](VkShaderStageFlags stages, uint32_t binding, VkDescriptorType type) {
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.pImmutableSamplers = nullptr; // Optional
			layoutBinding.stageFlags = stages;
			layoutBinding.descriptorType = type;
			return layoutBinding;
		};

		std::unordered_map<uint32_t, std::pair<VkShaderStageFlags, VkDescriptorType>> descriptors{};
		std::vector<VkPushConstantRange> pushConstants{};
		for (auto &shader : m_shaders)
		{
			for (const auto descriptor : shader.DescriptorTypes())
			{
				if (descriptors.find(descriptor.first) == descriptors.end())
				{
					descriptors[descriptor.first] =
					    std::make_pair(static_cast<VkShaderStageFlags>(shader.Stage()), descriptor.second);
				}
				else
				{
					descriptors[descriptor.first].first |= static_cast<VkShaderStageFlags>(shader.Stage());
				}
			}

			auto currentPushConstants = shader.PushConstants();
			pushConstants.insert(pushConstants.cend(), currentPushConstants.begin(), currentPushConstants.end());

			auto module = std::make_unique<ShaderModule>(m_logicalDevice, shader.ShaderModuleCreateInfo());
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
			shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageCreateInfo.stage = static_cast<VkShaderStageFlagBits>(shader.Stage());
			shaderStageCreateInfo.module = module->VkHandle();
			shaderStageCreateInfo.pName = "main";

			shaderModules.emplace_back(std::move(module));
			shaderStages.emplace_back(shaderStageCreateInfo);
		}

		bindings.reserve(descriptors.size());
		for (const auto &descriptor : descriptors)
			bindings.emplace_back(
			    buildDescriptorSetLayoutBinding(descriptor.second.first, descriptor.first, descriptor.second.second));

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_logicalDevice->VkHandle(), &layoutInfo, nullptr, &m_descriptorSetLayout) !=
		    VK_SUCCESS)
		{
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
		viewport.width = static_cast<float>(m_settings.swapChainExtent.width);
		viewport.height = static_cast<float>(m_settings.swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = m_settings.swapChainExtent;

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
		rasterizer.polygonMode = static_cast<VkPolygonMode>(m_settings.drawType);
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f;          // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;          // Optional
		multisampling.pSampleMask = nullptr;            // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE;      // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

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

		if (!pushConstants.empty())
		{
			pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
			pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
		}

		if (vkCreatePipelineLayout(m_logicalDevice->VkHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size();

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
		pipelineInfo.basePipelineIndex = -1;              // Optional

		if (vkCreateGraphicsPipelines(
		        m_logicalDevice->VkHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}
	}

	GraphicPipeline::GraphicPipeline(GraphicPipeline &&other) noexcept
	{
		m_logicalDevice = std::exchange(other.m_logicalDevice, nullptr);
		m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		m_pipelineLayout =
		    std::exchange(other.m_pipelineLayout, static_cast<decltype(m_pipelineLayout)>(VK_NULL_HANDLE));
		m_descriptorSetLayout =
		    std::exchange(other.m_descriptorSetLayout, static_cast<decltype(m_descriptorSetLayout)>(VK_NULL_HANDLE));

		std::swap(m_renderFunction, other.m_renderFunction);
		std::swap(m_renderPass, other.m_renderPass);
		std::swap(m_settings, other.m_settings);
	}

	GraphicPipeline &GraphicPipeline::operator=(GraphicPipeline &&other) noexcept
	{
		std::swap(m_logicalDevice, other.m_logicalDevice);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_pipelineLayout, other.m_pipelineLayout);
		std::swap(m_descriptorSetLayout, other.m_descriptorSetLayout);
		std::swap(m_renderPass, other.m_renderPass);
		std::swap(m_settings, other.m_settings);
		std::swap(m_renderFunction, other.m_renderFunction);

		return *this;
	}

	void GraphicPipeline::Bind(VkCommandBuffer commandsBuffer) const
	{
		vkCmdBindPipeline(commandsBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkHandle);
	}

	void GraphicPipeline::Configure(vzt::PipelineSettings settings)
	{
		m_settings = settings;
		m_renderPass = std::make_unique<vzt::RenderPass>(m_logicalDevice, m_settings.swapChainImageFormat);

		Cleanup();
		Create();
	}

	void GraphicPipeline::Render(VkCommandBuffer commandBuffer, uint32_t bufferNumber) const
	{
		if (m_renderFunction)
		{
			Bind(commandBuffer);
			m_renderFunction(commandBuffer, this, bufferNumber);
		}
	}

	void GraphicPipeline::SetRenderFunction(RenderFunction renderFunction)
	{
		m_renderFunction = renderFunction;
	}

	void GraphicPipeline::SetShader(Shader shader)
	{
		m_shaders.emplace(std::move(shader));
	}

	std::vector<VkDescriptorType> GraphicPipeline::DescriptorTypes() const
	{
		std::vector<VkDescriptorType> descriptorTypes;
		for (auto &shader : m_shaders)
		{
			auto types = shader.DescriptorTypes();
			for (const auto &type : types)
				descriptorTypes.emplace_back(type.second);
		}
		return descriptorTypes;
	}

	void GraphicPipeline::Cleanup()
	{
		if (m_descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(m_logicalDevice->VkHandle(), m_descriptorSetLayout, nullptr);
			m_descriptorSetLayout = VK_NULL_HANDLE;
		}

		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}

		if (m_pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(m_logicalDevice->VkHandle(), m_pipelineLayout, nullptr);
			m_pipelineLayout = VK_NULL_HANDLE;
		}
	}

	GraphicPipeline::~GraphicPipeline()
	{
		Cleanup();
	}
} // namespace vzt
