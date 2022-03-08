#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"

#include <iostream>
#include <numeric>
#include <stack>

namespace vzt
{
	RenderPassHandler::RenderPassHandler(std::string name, vzt::QueueType queueType)
	    : m_name(std::move(name)), m_queueType(queueType)
	{
	}

	void RenderPassHandler::addColorInput(const vzt::AttachmentHandle attachment, const std::string& attachmentName)
	{
		AttachmentInfo attachmentInfo;

		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ShaderReadOnlyOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Load;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		vzt::ImageBarrier barrier{};
		barrier.layout            = vzt::ImageLayout::ShaderReadOnlyOptimal;
		barrier.accesses          = vzt::AccessFlag::ShaderRead;
		barrier.stages            = vzt::PipelineStage::FragmentShader;
		attachmentInfo.barrier    = barrier;
		m_colorInputs[attachment] = attachmentInfo;
	}

	void RenderPassHandler::addColorOutput(vzt::AttachmentHandle& attachment, const std::string& attachmentName)
	{
		attachment.state++;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ColorAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Clear;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::Store;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_colorOutputs[attachment] = attachmentInfo;
	}

	void RenderPassHandler::addColorInputOutput(vzt::AttachmentHandle& attachment, const std::string& inName,
	                                            const std::string& outName)
	{
		attachment.state++;

		AttachmentInfo inAttachmentInfo{};
		inAttachmentInfo.name = inName;
		if (inAttachmentInfo.name.empty())
		{
			inAttachmentInfo.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());
		}

		inAttachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		inAttachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ColorAttachmentOptimal;
		inAttachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Load;
		inAttachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::Store;
		inAttachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		inAttachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		inAttachmentInfo.barrier = {vzt::PipelineStage::FragmentShader, vzt::AccessFlag::ShaderRead,
		                            vzt::ImageLayout::ShaderReadOnlyOptimal};

		m_colorInputs[attachment] = inAttachmentInfo;

		AttachmentInfo outAttachmentInfo{};
		outAttachmentInfo.name = outName;
		if (outAttachmentInfo.name.empty())
		{
			outAttachmentInfo.name = m_name + "ColorOut" + std::to_string(m_colorInputs.size());
		}

		m_colorOutputs[attachment] = outAttachmentInfo;
	}

	void RenderPassHandler::addStorageInput(const vzt::StorageHandle storage, const std::string& storageName)
	{
		StorageInfo inStorageInfo;
		inStorageInfo.name = storageName;
		if (inStorageInfo.name.empty())
		{
			inStorageInfo.name = m_name + "StorageIn" + std::to_string(m_storageInputs.size());
		}

		vzt::StorageBarrier barrier{};
		barrier.accesses = vzt::AccessFlag::ShaderRead;
		if (m_queueType == vzt::QueueType::Compute)
		{
			barrier.stages = vzt::PipelineStage::ComputeShader;
		}
		else
		{
			barrier.stages = vzt::PipelineStage::FragmentShader;
		}
		inStorageInfo.barrier    = barrier;
		m_storageInputs[storage] = inStorageInfo;
	}

	void RenderPassHandler::addStorageOutput(vzt::StorageHandle& storage, const std::string& storageName)
	{
		storage.state++;
		StorageInfo inStorageInfo{};
		inStorageInfo.name = storageName;
		if (inStorageInfo.name.empty())
		{
			inStorageInfo.name = m_name + "StorageOut" + std::to_string(m_storageInputs.size());
		}

		m_storageOutputs[storage] = inStorageInfo;
	}

	void RenderPassHandler::addStorageInputOutput(vzt::StorageHandle& storage, const std::string& inName,
	                                              const std::string& outName)
	{
		storage.state++;

		StorageInfo inStorageInfo{};
		inStorageInfo.name = inName;
		if (inStorageInfo.name.empty())
		{
			inStorageInfo.name = m_name + "In" + std::to_string(m_colorInputs.size());
		}

		vzt::StorageBarrier barrier{};
		barrier.accesses = vzt::AccessFlag::ShaderRead | vzt::AccessFlag::ShaderWrite;
		if (m_queueType == vzt::QueueType::Compute)
		{
			barrier.stages = vzt::PipelineStage::ComputeShader;
		}
		else
		{
			barrier.stages = vzt::PipelineStage::FragmentShader;
		}
		inStorageInfo.barrier    = barrier;
		m_storageInputs[storage] = inStorageInfo;

		StorageInfo outAttachmentInfo{};
		outAttachmentInfo.name = outName;
		if (outAttachmentInfo.name.empty())
		{
			outAttachmentInfo.name = m_name + "Out" + std::to_string(m_colorInputs.size());
		}

		m_storageOutputs[storage] = outAttachmentInfo;
	}

	void RenderPassHandler::setDepthStencilInput(const vzt::AttachmentHandle depthStencil,
	                                             const std::string&          attachmentName)
	{
		DepthAttachmentInfo attachmentInfo;
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "DepthIn";
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_depthInput = {depthStencil, attachmentInfo};
	}

	void RenderPassHandler::setDepthStencilOutput(vzt::AttachmentHandle& depthStencil,
	                                              const std::string&     attachmentName)
	{
		depthStencil.state++;

		DepthAttachmentInfo attachmentInfo;
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "DepthOut";
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_depthOutput = {depthStencil, attachmentInfo};
	}

	void RenderPassHandler::setRenderFunction(vzt::RecordFunction recordFunction)
	{
		m_recordFunction = std::move(recordFunction);
	}

	void RenderPassHandler::setConfigureFunction(vzt::ConfigureFunction configureFunction)
	{
		m_configureFunction = std::move(configureFunction);
	}

	void RenderPassHandler::setDepthClearFunction(vzt::DepthClearFunction depthClearFunction)
	{
		m_depthClearFunction = std::move(depthClearFunction);
	}

	void RenderPassHandler::setColorClearFunction(vzt::ColorClearFunction colorClearFunction)
	{
		m_colorClearFunction = std::move(colorClearFunction);
	}

	bool RenderPassHandler::isDependingOn(const RenderPassHandler& other) const
	{
		for (const auto& output : m_colorOutputs)
		{
			const auto currentHandle = output.first;
			for (const auto& input : other.m_colorInputs)
			{
				if (input.first.id == currentHandle.id && input.first.state == currentHandle.state)
				{
					return true;
				}
			}
		}

		for (const auto& output : m_storageOutputs)
		{
			const auto currentHandle = output.first;
			for (const auto& input : other.m_storageOutputs)
			{
				if (input.first.id == currentHandle.id && input.first.state == currentHandle.state)
				{
					return true;
				}
			}
		}

		// Depth attachment
		if (m_depthOutput.has_value())
		{
			const auto currentHandle = m_depthOutput.value().first;
			if (other.m_depthInput.has_value())
			{
				const auto otherHandle = other.m_depthInput.value().first;
				if (otherHandle.id == currentHandle.id && otherHandle.state == currentHandle.state)
				{
					return true;
				}
			}
		}

		return false;
	}

	std::unique_ptr<vzt::RenderPass> RenderPassHandler::build(RenderGraph* const correspondingGraph,
	                                                          const vzt::Device* device, const uint32_t imageId,
	                                                          const uint32_t               imageCount,
	                                                          const vzt::Size2D<uint32_t>& targetSize,
	                                                          const vzt::Format            targetFormat)
	{
		if (!m_recordFunction)
		{
			throw std::runtime_error("A render pass must have a render function");
		}

		std::vector<vzt::RenderPass::AttachmentPassConfiguration> attachments;
		attachments.reserve(m_colorOutputs.size());
		uint32_t i = 0;
		for (const auto& output : m_colorOutputs)
		{
			auto attachmentUse = output.second.attachmentUse;
			if (correspondingGraph->isBackBuffer(output.first))
			{
				attachmentUse.finalLayout = vzt::ImageLayout::PresentSrcKHR;
			}

			if (m_colorClearFunction)
				m_colorClearFunction(i++, &attachmentUse.clearValue);
			attachments.emplace_back(correspondingGraph->getAttachment(imageId, output.first), attachmentUse);
		}

		std::unique_ptr<vzt::RenderPass> renderPass{};
		if (m_depthOutput.has_value())
		{
			auto depthAttachment = m_depthOutput.value();
			if (m_depthClearFunction)
				m_depthClearFunction(&depthAttachment.second.attachmentUse.clearValue);

			vzt::RenderPass::DepthAttachmentPassConfiguration depthUse = {
			    correspondingGraph->getAttachment(imageId, depthAttachment.first),
			    depthAttachment.second.attachmentUse};
			renderPass = std::make_unique<vzt::RenderPass>(device, attachments, depthUse);
		}
		else if (m_queueType == vzt::QueueType::Graphic)
		{
			throw std::runtime_error("A graphic render pass should have a depth attachment.");
		}

		if (!m_descriptorPool && !m_colorInputs.empty())
		{
			vzt::DescriptorLayout currentLayout{};
			for (std::size_t i = 0; i < m_colorInputs.size(); i++)
			{
				currentLayout.addBinding(vzt::ShaderStage::FragmentShader, static_cast<uint32_t>(i),
				                         vzt::DescriptorType::CombinedSampler);
			}
			currentLayout.configure(device);

			m_descriptorPool =
			    std::make_unique<vzt::DescriptorPool>(device, std::vector{vzt::DescriptorType::CombinedSampler});
			m_descriptorPool->allocate(3, currentLayout);
			m_descriptorLayout = std::move(currentLayout);
		}

		if (imageId == 0 && m_configureFunction)
		{
			std::vector<const vzt::DescriptorLayout*> descriptors{};
			if (m_descriptorLayout.has_value())
				descriptors.emplace_back(&m_descriptorLayout.value());

			m_configureFunction(imageCount, {device, renderPass.get(), descriptors});
		}

		if (!m_colorInputs.empty())
		{
			auto inputStart = m_colorInputs.begin();

			IndexedUniform<vzt::Texture*> texturesDescriptors;
			for (uint32_t i = 0; i < m_colorInputs.size(); i++)
			{
				texturesDescriptors[i] = correspondingGraph->getAttachment(imageId, inputStart->first)->asTexture();
				inputStart++;
			}
			m_descriptorPool->update(imageId, texturesDescriptors);
		}

		return renderPass;
	}

	void RenderPassHandler::render(const uint32_t imageId, const vzt::RenderPass* renderPass,
	                               VkCommandBuffer commandBuffer) const
	{
		std::vector<VkDescriptorSet> descriptorSets;
		if (m_descriptorPool)
		{
			descriptorSets.emplace_back((*m_descriptorPool)[imageId]);
		}

		m_recordFunction(imageId, renderPass, commandBuffer, descriptorSets);
	}

	RenderGraph::RenderGraph() = default;
	RenderGraph::~RenderGraph()
	{
		for (auto& semaphores : m_semaphores)
		{
			for (auto& semaphore : semaphores)
			{
				vkDestroySemaphore(m_device->vkHandle(), semaphore, nullptr);
			}
		}
	};

	vzt::AttachmentHandle RenderGraph::addAttachment(const vzt::AttachmentSettings& settings)
	{
		const vzt::AttachmentHandle handle = generateAttachmentHandle();
		m_attachmentsSettings[handle]      = settings;

		return handle;
	}

	vzt::StorageHandle RenderGraph::addStorage(const vzt::StorageSettings& settings)
	{
		const vzt::StorageHandle handle = generateStorageHandle();
		m_storagesSettings[handle]      = settings;

		return handle;
	}

	vzt::RenderPassHandler& RenderGraph::addPass(const std::string& name, const vzt::QueueType queueType)
	{
		m_renderPassHandlers.emplace_back(vzt::RenderPassHandler(name, queueType));
		return m_renderPassHandlers.back();
	}

	void RenderGraph::setBackBuffer(const vzt::AttachmentHandle backBufferHandle) { m_backBuffer = backBufferHandle; }
	bool RenderGraph::isBackBuffer(const vzt::AttachmentHandle backBufferHandle) const
	{
		return m_backBuffer == backBufferHandle;
	}

	void RenderGraph::compile()
	{
		sortRenderPasses();

		std::cout << "Sorted render passes :" << std::endl;
		for (std::size_t i = 0; i < m_sortedRenderPassIndices.size(); i++)
		{
			std::cout << std::to_string(i) << " => " << m_renderPassHandlers[m_sortedRenderPassIndices[i]].m_name
			          << std::endl;
		}
		std::cout << std::endl;

		reorderRenderPasses();

		std::cout << "Reordered render passes :" << std::endl;
		for (std::size_t i = 0; i < m_sortedRenderPassIndices.size(); i++)
		{
			std::cout << std::to_string(i) << " => " << m_renderPassHandlers[m_sortedRenderPassIndices[i]].m_name
			          << std::endl;
		}
	}

	void RenderGraph::configure(const vzt::Device* device, const std::vector<VkImage>& swapchainImages,
	                            vzt::Size2D<uint32_t> scImageSize, vzt::Format scColorFormat, vzt::Format scDepthFormat)
	{
		m_device = device;

		m_attachmentsIndices.resize(swapchainImages.size());
		m_frameBuffers.resize(swapchainImages.size());
		m_semaphores.resize(swapchainImages.size());

		m_attachments.clear();
		m_attachments.reserve(m_attachmentsSettings.size() * swapchainImages.size());
		m_commandPools.reserve(swapchainImages.size());

		uint32_t              semaphoreCount = 0;
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (uint32_t i = 0; i < swapchainImages.size(); i++)
		{
			auto swapchainImage = swapchainImages[i];
			m_commandPools.emplace_back(device);

			// Create attachments
			for (const auto& attachmentSettings : m_attachmentsSettings)
			{
				const bool isFinalImage =
				    m_backBuffer.value_or(vzt::AttachmentHandle{~0ULL, 0}).id == attachmentSettings.first.id;
				vzt::Format currentFormat = attachmentSettings.second.format.value_or(scColorFormat);

				const bool isDepthStencil =
				    (attachmentSettings.second.usage & vzt::ImageUsage::DepthStencilAttachment) ==
				    vzt::ImageUsage::DepthStencilAttachment;
				if (!isFinalImage && isDepthStencil)
				{
					currentFormat = scDepthFormat;
				}

				std::unique_ptr<vzt::Attachment> attachment;
				if (isFinalImage)
				{
					attachment = std::make_unique<vzt::Attachment>(device, swapchainImage, scColorFormat,
					                                               vzt::ImageLayout::ColorAttachmentOptimal,
					                                               vzt::ImageAspect::Color);
				}
				else
				{
					attachment = std::make_unique<vzt::Attachment>(
					    device, attachmentSettings.second.imageSize.value_or(scImageSize), currentFormat,
					    attachmentSettings.second.usage);
				}

				m_attachmentsIndices[i][attachmentSettings.first] = m_attachments.size();
				m_attachments.emplace_back(std::move(attachment));
			}

			// TODO: Create storages too

			m_frameBuffers[i].reserve(m_renderPassHandlers.size());
			for (const std::size_t sortedRenderPassIndice : m_sortedRenderPassIndices)
			{
				auto& renderPassHandler = m_renderPassHandlers[sortedRenderPassIndice];
				auto  renderPass        = renderPassHandler.build(
				            this, device, i, static_cast<uint32_t>(swapchainImages.size()), scImageSize, scColorFormat);
				std::vector<const vzt::ImageView*> views;
				views.reserve(renderPassHandler.m_colorOutputs.size());

				if (i == 0)
				{
					for (const auto& colorInput : renderPassHandler.m_colorInputs)
					{
						if (m_attachmentsSynchronizations[colorInput.first].size() <= colorInput.first.state)
						{
							m_attachmentsSynchronizations[colorInput.first].emplace_back(semaphoreCount);
							semaphoreCount++;
						}
					}
				}

				for (const auto& colorOutput : renderPassHandler.m_colorOutputs)
				{
					views.emplace_back(m_attachments[m_attachmentsIndices[i][colorOutput.first]]->getView());
				}

				if (renderPassHandler.m_depthOutput.has_value())
				{
					views.emplace_back(
					    m_attachments[m_attachmentsIndices[i][renderPassHandler.m_depthOutput.value().first]]
					        ->getView());
				}

				m_frameBuffers[i].emplace_back(
				    std::make_unique<vzt::FrameBuffer>(device, std::move(renderPass), scImageSize, views));
			}

			m_commandPools[i].allocateCommandBuffers(static_cast<uint32_t>(m_frameBuffers[i].size()));
			m_semaphores[i].resize(semaphoreCount);

			for (std::size_t s = 0; s < semaphoreCount; s++)
			{
				if (vkCreateSemaphore(m_device->vkHandle(), &semaphoreCreateInfo, nullptr, &m_semaphores[i][s]) !=
				    VK_SUCCESS)
				{
					throw std::runtime_error("Can't create semaphores");
				}
			}
		}
	}

	void RenderGraph::render(const std::size_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete,
	                         VkFence inFlightFence)
	{
		const auto& frameBuffers = m_frameBuffers[imageId];
		const auto& semaphores   = m_semaphores[imageId];

		std::vector<VkSubmitInfo> graphicSubmissions{};
		graphicSubmissions.reserve(frameBuffers.size());
		for (uint32_t i = 0; i < frameBuffers.size(); i++)
		{
			const std::size_t renderPassIdx = m_sortedRenderPassIndices[i];
			const auto&       currentFb     = frameBuffers[renderPassIdx];
			const auto&       renderPass    = m_renderPassHandlers[renderPassIdx];

			m_commandPools[imageId].recordBuffer(i, [&](VkCommandBuffer commandBuffer) {
				currentFb->bind(commandBuffer);
				renderPass.render(i, currentFb->getRenderPass(), commandBuffer);
				currentFb->unbind(commandBuffer);
			});

			VkSubmitInfo submitInfo{};
			submitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
			submitInfo.pWaitDstStageMask      = waitStages;

			std::vector<VkSemaphore> waitSemaphores{};
			waitSemaphores.reserve(renderPass.m_colorInputs.size());
			if (i == 0)
			{
				waitSemaphores.emplace_back(imageAvailable);
			}

			for (const auto& colorInput : renderPass.m_colorInputs)
			{
				const std::size_t semaphoreId = m_attachmentsSynchronizations[colorInput.first][colorInput.first.state];
				waitSemaphores.emplace_back(semaphores[semaphoreId]);
			}

			submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
			submitInfo.pWaitSemaphores    = waitSemaphores.data();

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers    = &m_commandPools[imageId][i];

			std::vector<VkSemaphore> signalSemaphores{};
			signalSemaphores.reserve(renderPass.m_colorOutputs.size());
			if (i == frameBuffers.size() - 1)
			{
				signalSemaphores.emplace_back(renderComplete);
			}

			for (const auto& colorOutput : renderPass.m_colorOutputs)
			{
				const std::size_t semaphoreId =
				    m_attachmentsSynchronizations[colorOutput.first][colorOutput.first.state];
				signalSemaphores.emplace_back(semaphores[semaphoreId]);
			}

			submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
			submitInfo.pSignalSemaphores    = signalSemaphores.data();

			const VkQueue currentQueue = renderPass.m_queueType == vzt::QueueType::Graphic
			                                 ? m_device->getGraphicsQueue()
			                                 : throw std::runtime_error("Compute pipeline is not currently supported");

			graphicSubmissions.emplace_back(submitInfo);
		}

		vkResetFences(m_device->vkHandle(), 1, &inFlightFence);
		if (vkQueueSubmit(m_device->getGraphicsQueue(), static_cast<uint32_t>(graphicSubmissions.size()),
		                  graphicSubmissions.data(), VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit command buffers!");
		}
	}

	vzt::Attachment* RenderGraph::getAttachment(const std::size_t imageId, const vzt::AttachmentHandle& handle) const
	{
		auto it = m_attachmentsIndices[imageId].find(handle);
		if (it != m_attachmentsIndices[imageId].end())
			return m_attachments[it->second].get();
		throw std::runtime_error("Unknown handle");
	}

	const vzt::AttachmentSettings& RenderGraph::getAttachmentSettings(const vzt::AttachmentHandle& handle) const
	{
		for (const auto& savedHandle : m_attachmentsSettings)
		{
			if (savedHandle.first.id == handle.id)
			{
				return savedHandle.second;
			}
		}
		throw std::runtime_error("Unknown referenced attachment.");
	}

	const vzt::StorageSettings& RenderGraph::getStorageSettings(const vzt::StorageHandle& handle) const
	{
		for (const auto& savedHandle : m_storagesSettings)
		{
			if (savedHandle.first.id == handle.id)
			{
				return savedHandle.second;
			}
		}
		throw std::runtime_error("Unknown referenced storage.");
	}

	void RenderGraph::sortRenderPasses()
	{
		// https://en.wikipedia.org/wiki/Topological_sorting
		m_sortedRenderPassIndices.clear();
		m_sortedRenderPassIndices.reserve(m_sortedRenderPassIndices.size());

		// 0: unmarked, 1: temporary marked, 2: permanent mark
		std::vector<std::size_t> nodeStatus     = std::vector<std::size_t>(m_renderPassHandlers.size(), 0);
		std::vector<std::size_t> remainingNodes = std::vector<std::size_t>(m_renderPassHandlers.size());
		std::iota(remainingNodes.begin(), remainingNodes.end(), 0);

		std::function<void(std::size_t)> processNode;
		processNode = [&](std::size_t idx) {
			const std::size_t currentStatus = nodeStatus[idx];

			if (currentStatus == 1)
				throw std::runtime_error("The render graph is cyclic.");
			if (currentStatus == 2)
				return;

			nodeStatus[idx] = 1;

			const auto& currentRenderPass = m_renderPassHandlers[idx];
			for (std::size_t j = 0; j < m_renderPassHandlers.size(); j++)
			{
				if (j == idx)
					continue;

				const auto& renderPass = m_renderPassHandlers[j];
				if (currentRenderPass.isDependingOn(renderPass))
				{
					processNode(j);
				}
			}

			nodeStatus[idx] = 2;
			m_sortedRenderPassIndices.emplace(m_sortedRenderPassIndices.begin(), idx);
			remainingNodes.erase(std::remove(remainingNodes.begin(), remainingNodes.end(), idx), remainingNodes.end());
		};

		while (!remainingNodes.empty())
		{
			processNode(remainingNodes.front());
		}
	}

	void RenderGraph::reorderRenderPasses()
	{
		if (m_sortedRenderPassIndices.size() >= 2)
			return;

		// Based on https://github.com/Themaister/Granite/blob/master/renderer/render_graph.cpp#L2897
		// Expecting that m_sortedRenderPassIndices contains
		// the sorted list of render pass indices.
		std::vector<std::size_t> toProcess;
		toProcess.reserve(m_sortedRenderPassIndices.size());
		std::swap(toProcess, m_sortedRenderPassIndices);

		const auto schedule = [&](unsigned index) {
			// Need to preserve the order of remaining elements.
			m_sortedRenderPassIndices.push_back(toProcess[index]);
			toProcess.erase(toProcess.begin() + index);
		};

		schedule(0);
		while (!toProcess.empty())
		{
			std::size_t bestCandidateIdx  = 0;
			std::size_t bestOverlapFactor = 0;

			for (std::size_t i = 0; i < toProcess.size(); i++)
			{
				std::size_t overlapFactor = 0;
				// Try to find the farthest non-depending pass
				for (auto it = m_sortedRenderPassIndices.rbegin(); it != m_sortedRenderPassIndices.rend(); ++it)
				{
					if (m_renderPassHandlers[toProcess[i]].isDependingOn(m_renderPassHandlers[*it]))
						break;
					overlapFactor++;
				}

				if (overlapFactor <= bestOverlapFactor)
					continue;

				bool possibleCandidate = true;
				for (std::size_t j = 0; j < i; j++)
				{
					if (m_renderPassHandlers[toProcess[i]].isDependingOn(m_renderPassHandlers[toProcess[j]]))
					{
						possibleCandidate = false;
						break;
					}
				}

				if (!possibleCandidate)
					continue;

				bestCandidateIdx  = i;
				bestOverlapFactor = overlapFactor;
			}

			m_sortedRenderPassIndices.push_back(toProcess[bestCandidateIdx]);
			toProcess.erase(toProcess.begin() + bestCandidateIdx);
		}
	}

	vzt::AttachmentHandle RenderGraph::generateAttachmentHandle() const { return {m_hash(m_handleCounter++), 0}; }
	vzt::StorageHandle    RenderGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), 0}; }

} // namespace vzt
