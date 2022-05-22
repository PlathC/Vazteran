#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"

#include <algorithm>
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

		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ShaderReadOnlyOptimal;
		attachmentInfo.attachmentUse.usedLayout     = vzt::ImageLayout::ShaderReadOnlyOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Load;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		vzt::ImageBarrier barrier{};
		barrier.layout            = vzt::ImageLayout::ShaderReadOnlyOptimal;
		barrier.accesses          = vzt::AttachmentAccess::ShaderRead;
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

		attachmentInfo.attachmentUse.usedLayout     = vzt::ImageLayout::ColorAttachmentOptimal;
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

		inAttachmentInfo.attachmentUse.usedLayout     = vzt::ImageLayout::ShaderReadOnlyOptimal;
		inAttachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ShaderReadOnlyOptimal;
		inAttachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Load;
		inAttachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::Store;
		inAttachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		inAttachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		inAttachmentInfo.barrier = {vzt::PipelineStage::FragmentShader, vzt::AttachmentAccess::ShaderRead,
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
		barrier.accesses = vzt::AttachmentAccess::ShaderRead;
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
		barrier.accesses = vzt::AttachmentAccess::ShaderRead | vzt::AttachmentAccess::ShaderWrite;
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
		AttachmentInfo attachmentInfo;
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "DepthIn";
		}

		// attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.usedLayout     = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Load;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::Load;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_depthInput = {depthStencil, attachmentInfo};
	}

	void RenderPassHandler::setDepthStencilOutput(vzt::AttachmentHandle& depthStencil,
	                                              const std::string&     attachmentName)
	{
		depthStencil.state++;

		AttachmentInfo attachmentInfo;
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "DepthOut";
		}

		attachmentInfo.attachmentUse.usedLayout     = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Clear;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::Store;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::Clear;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::Store;

		m_depthOutput = {depthStencil, attachmentInfo};
	}

	void RenderPassHandler::setConfigureFunction(ConfigureFunction configureFunction)
	{
		m_configureFunction = std::move(configureFunction);
	}

	void RenderPassHandler::setRecordFunction(vzt::RecordFunction recordFunction)
	{
		m_recordFunction = std::move(recordFunction);
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

	void RenderPassHandler::configure(RenderGraph* correspondingGraph, const Device* device, const uint32_t imageCount,
	                                  vzt::Size2D<uint32_t> targetSize)
	{
		if (!m_recordFunction)
			throw std::runtime_error("A render pass must have a record function");

		m_configuration = {};

		m_extent = std::move(targetSize);
		m_device = device;
		m_parent = correspondingGraph;

		m_configuration.colorAttachments.reserve(m_colorOutputs.size());
		uint32_t outputIndex = 0;
		for (auto& output : m_colorOutputs)
		{
			auto& attachmentUse = output.second.attachmentUse;

			if (m_colorClearFunction)
			{
				attachmentUse.clearValue = {};
				m_colorClearFunction(outputIndex++, &(*attachmentUse.clearValue));
			}

			const AttachmentSettings& settings = m_parent->getAttachmentSettings(output.first);

			attachmentUse.format      = *settings.format;
			attachmentUse.sampleCount = settings.sampleCount;
			m_configuration.colorAttachments.emplace_back(attachmentUse);
		}

		if (!m_colorOutputs.empty())
		{
			m_configuration.dependencies.emplace_back();
			SubpassDependency& incomingDependency = m_configuration.dependencies.back();

			incomingDependency.src       = SubpassDependency::ExternalSubpass;
			incomingDependency.dst       = 0;
			incomingDependency.srcStage  = vzt::PipelineStage::BottomOfPipe;
			incomingDependency.srcAccess = vzt::AttachmentAccess::MemoryRead;
			incomingDependency.dstStage  = vzt::PipelineStage::ColorAttachmentOutput;
			incomingDependency.dstAccess =
			    vzt::AttachmentAccess::ColorAttachmentWrite | vzt::AttachmentAccess::ColorAttachmentRead;

			m_configuration.dependencies.emplace_back();
			SubpassDependency& outcomingDependency = m_configuration.dependencies.back();

			outcomingDependency.src      = 0;
			outcomingDependency.dst      = SubpassDependency::ExternalSubpass;
			outcomingDependency.srcStage = vzt::PipelineStage::ColorAttachmentOutput;
			outcomingDependency.srcAccess =
			    vzt::AttachmentAccess::ColorAttachmentWrite | vzt::AttachmentAccess::ColorAttachmentRead;
			outcomingDependency.dstStage  = vzt::PipelineStage::BottomOfPipe;
			outcomingDependency.dstAccess = vzt::AttachmentAccess::MemoryRead;
		}

		std::unique_ptr<vzt::RenderPass> renderPass{};
		if (m_depthOutput)
		{
			auto& [handle, info] = *m_depthOutput;
			if (m_depthClearFunction)
			{
				info.attachmentUse.clearValue = {};
				m_depthClearFunction(&(*info.attachmentUse.clearValue));
			}

			m_configuration.depthAttachment    = info.attachmentUse;
			const AttachmentSettings& settings = m_parent->getAttachmentSettings(handle);

			m_configuration.depthAttachment.format      = *settings.format;
			m_configuration.depthAttachment.sampleCount = settings.sampleCount;

			m_configuration.dependencies.emplace_back();
			SubpassDependency& dependency = m_configuration.dependencies.back();

			dependency.src       = SubpassDependency::ExternalSubpass;
			dependency.dst       = 0;
			dependency.srcStage  = vzt::PipelineStage::Transfer;
			dependency.srcAccess = vzt::AttachmentAccess::TransferWrite;
			if (m_depthOutput->first.state > 1)
			{
				dependency.srcStage = dependency.srcStage | vzt::PipelineStage::EarlyFragmentTests |
				                      vzt::PipelineStage::LateFragmentTests;
				dependency.srcAccess = dependency.srcAccess | vzt::AttachmentAccess::DepthStencilAttachmentWrite;
			}
			dependency.dstStage = vzt::PipelineStage::EarlyFragmentTests | vzt::PipelineStage::LateFragmentTests;
			dependency.dstAccess =
			    vzt::AttachmentAccess::DepthStencilAttachmentRead | vzt::AttachmentAccess::DepthStencilAttachmentWrite;
		}
		else if (m_queueType == vzt::QueueType::Graphic)
		{
			throw std::runtime_error("A graphic render pass should have a depth attachment.");
		}

		if (!m_descriptorPool && !m_colorInputs.empty())
		{
			m_descriptorLayout = vzt::DescriptorLayout{};
			for (std::size_t i = 0; i < m_colorInputs.size(); i++)
			{
				m_descriptorLayout->addBinding(static_cast<uint32_t>(i), vzt::DescriptorType::CombinedSampler);
			}
			m_descriptorLayout->configure(device);

			m_descriptorPool =
			    std::make_unique<vzt::DescriptorPool>(device, std::vector{vzt::DescriptorType::CombinedSampler});
			m_descriptorPool->allocate(imageCount, *m_descriptorLayout);
		}
	}

	std::unique_ptr<vzt::RenderPass> RenderPassHandler::build(const uint32_t imageId)
	{
		if (!m_colorInputs.empty())
		{
			IndexedUniform<vzt::Texture*> texturesDescriptors;
			auto                          input = m_colorInputs.begin();
			for (uint32_t i = 0; i < m_colorInputs.size(); ++input, i++)
			{
				const Attachment& attachment = m_parent->getAttachment(imageId, input->first);
				texturesDescriptors[i]       = attachment.asTexture();
			}
			m_descriptorPool->update(imageId, texturesDescriptors);
		}

		auto renderPass = std::make_unique<vzt::RenderPass>(m_device, m_configuration);
		if (imageId == 0)
		{
			m_renderPassTemplate = renderPass.get();
			m_configureFunction(*this);
		}

		return renderPass;
	}

	void RenderPassHandler::render(const uint32_t imageId, VkCommandBuffer commandBuffer) const
	{
		std::vector<VkDescriptorSet> descriptorSets;
		if (m_descriptorPool)
			descriptorSets.emplace_back((*m_descriptorPool)[imageId]);

		m_recordFunction(imageId, commandBuffer, descriptorSets);
	}

	RenderGraph::RenderGraph()  = default;
	RenderGraph::~RenderGraph() = default;

	vzt::AttachmentHandle RenderGraph::addAttachment(AttachmentSettings settings)
	{
		const vzt::AttachmentHandle handle = generateAttachmentHandle();
		m_attachmentsSettings[handle]      = std::move(settings);

		return handle;
	}

	vzt::StorageHandle RenderGraph::addStorage(StorageSettings settings)
	{
		const vzt::StorageHandle handle = generateStorageHandle();
		m_storagesSettings[handle]      = std::move(settings);

		return handle;
	}

	vzt::RenderPassHandler& RenderGraph::addPass(const std::string& name, const vzt::QueueType queueType)
	{
		m_renderPassHandlers.emplace_back(RenderPassHandler{name, queueType});
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
		reorderRenderPasses();

		// Resolve attachment layouts
		AttachmentList<ImageLayout> lastAttachmentsLayout{};
		for (const auto& [id, _] : m_attachmentsSettings)
			lastAttachmentsLayout[id] = vzt::ImageLayout::Undefined;

		for (const std::size_t sortedRenderPassIndice : m_sortedRenderPassIndices)
		{
			auto& renderPassHandler = m_renderPassHandlers[sortedRenderPassIndice];
			for (auto& [id, info] : renderPassHandler.m_colorOutputs)
			{
				info.attachmentUse.initialLayout = lastAttachmentsLayout[id];
				if (isBackBuffer(id))
					info.attachmentUse.finalLayout = ImageLayout::PresentSrcKHR;

				lastAttachmentsLayout[id] = info.attachmentUse.finalLayout;
			}

			if (renderPassHandler.m_depthOutput)
			{
				auto& [id, info]                 = *renderPassHandler.m_depthOutput;
				info.attachmentUse.initialLayout = lastAttachmentsLayout[id];
				lastAttachmentsLayout[id]        = info.attachmentUse.finalLayout;
			}

			for (auto& [id, info] : renderPassHandler.m_colorInputs)
				info.attachmentUse.initialLayout = lastAttachmentsLayout[id];

			if (renderPassHandler.m_depthInput)
			{
				auto& [id, info]                 = *renderPassHandler.m_depthInput;
				info.attachmentUse.initialLayout = lastAttachmentsLayout[id];
			}
		}
	}

	void RenderGraph::configure(const Device* device, const std::vector<VkImage>& swapchainImages,
	                            Size2D<uint32_t> scImageSize, Format scColorFormat, Format scDepthFormat)
	{
		m_device = device;

		m_attachmentsIndices.clear();
		m_attachmentsIndices.resize(swapchainImages.size());
		m_frameBuffers.clear();
		m_frameBuffers.resize(swapchainImages.size());

		m_attachments.clear();
		m_attachments.reserve(m_attachmentsSettings.size() * swapchainImages.size());
		m_commandPools.reserve(swapchainImages.size());

		for (auto& [handle, settings] : m_attachmentsSettings)
		{
			const bool isFinalImage   = m_backBuffer && m_backBuffer->id == handle.id;
			const bool isDepthStencil = static_cast<bool>(settings.usage & vzt::ImageUsage::DepthStencilAttachment);

			const Format format = settings.format.value_or(isDepthStencil ? scDepthFormat : scColorFormat);
			settings.format     = isFinalImage ? scColorFormat : format;

			if (!settings.imageSize || *settings.imageSize == m_lastScImageSize)
				settings.imageSize = scImageSize;
		}

		for (RenderPassHandler& pass : m_renderPassHandlers)
			pass.configure(this, device, static_cast<uint32_t>(swapchainImages.size()), scImageSize);

		m_lastScImageSize = std::move(scImageSize);

		for (uint32_t i = 0; i < swapchainImages.size(); i++)
		{
			auto swapchainImage = swapchainImages[i];
			m_commandPools.emplace_back(device);

			auto& attachmentIndices = m_attachmentsIndices[i];

			// Create attachments
			for (const auto& [id, settings] : m_attachmentsSettings)
			{
				attachmentIndices[id] = m_attachments.size();
				if (isBackBuffer(id))
				{
					m_attachments.emplace_back(device, swapchainImage, *settings.format,
					                           ImageLayout::ColorAttachmentOptimal, ImageAspect::Color);
				}
				else
				{
					m_attachments.emplace_back(device, *settings.imageSize, *settings.format, settings.usage);
				}
			}

			// TODO: Create storages too

			auto& framebuffers = m_frameBuffers[i];
			framebuffers.reserve(m_renderPassHandlers.size());
			for (const std::size_t sortedRenderPassIndice : m_sortedRenderPassIndices)
			{
				auto& renderPassHandler = m_renderPassHandlers[sortedRenderPassIndice];

				std::vector<const ImageView*> views;
				views.reserve(renderPassHandler.m_colorOutputs.size());

				for (auto& [id, _] : renderPassHandler.m_colorOutputs)
					views.emplace_back(m_attachments[attachmentIndices[id]].getView());

				if (renderPassHandler.m_depthOutput)
				{
					auto& [id, _] = *renderPassHandler.m_depthOutput;
					views.emplace_back(m_attachments[attachmentIndices[id]].getView());
				}

				framebuffers.emplace_back(device, renderPassHandler.build(i), scImageSize, views);
			}

			m_commandPools[i].allocateCommandBuffers(static_cast<uint32_t>(m_frameBuffers[i].size()));
		}
	}

	void RenderGraph::render(const std::size_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete,
	                         VkFence inFlightFence)
	{
		const auto& frameBuffers = m_frameBuffers[imageId];

		std::vector<VkSubmitInfo> graphicSubmissions{};
		graphicSubmissions.reserve(frameBuffers.size());

		for (uint32_t i = 0; i < frameBuffers.size(); i++)
		{
			const std::size_t              renderPassIdx = m_sortedRenderPassIndices[i];
			const auto&                    frameBuffer   = frameBuffers[renderPassIdx];
			const auto&                    renderPass    = m_renderPassHandlers[renderPassIdx];
			constexpr VkPipelineStageFlags waitStages    = toVulkan(PipelineStage::ColorAttachmentOutput);

			bool isPresentationPass = false;
			for (const auto& [id, _] : renderPass.m_colorOutputs)
			{
				if (isBackBuffer(id))
					isPresentationPass = true;
			}

			m_commandPools[imageId].recordBuffer(i, [&](VkCommandBuffer commandBuffer) {
				for (const auto& [id, info] : renderPass.m_colorInputs)
				{
					if (isBackBuffer(id))
						return;

					const Attachment& attachment = getAttachment(imageId, id);

					VkImageMemoryBarrier inputBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
					inputBarrier.oldLayout     = vzt::toVulkan(info.attachmentUse.initialLayout);
					inputBarrier.newLayout     = vzt::toVulkan(info.attachmentUse.usedLayout);
					inputBarrier.dstAccessMask = toVulkan(AttachmentAccess::ShaderRead);
					inputBarrier.image         = attachment.getView()->image();

					PipelineStage inputStage;
					if (attachment.getLayout() == ImageLayout::DepthStencilAttachmentOptimal)
					{
						inputStage                    = PipelineStage::LateFragmentTests;
						inputBarrier.subresourceRange = {toVulkan(ImageAspect::Depth), 0, 1, 0, 1};
						inputBarrier.srcAccessMask    = toVulkan(AttachmentAccess::DepthStencilAttachmentWrite);
					}
					else
					{
						inputStage                    = PipelineStage::ColorAttachmentOutput;
						inputBarrier.subresourceRange = {toVulkan(ImageAspect::Color), 0, 1, 0, 1};
						inputBarrier.srcAccessMask    = toVulkan(AttachmentAccess::ColorAttachmentWrite);
					}

					vkCmdPipelineBarrier(commandBuffer, toVulkan(inputStage), toVulkan(PipelineStage::FragmentShader),
					                     toVulkan(DependencyFlag::ByRegion), 0, nullptr, 0, nullptr, 1, &inputBarrier);
				}

				frameBuffer.bind(commandBuffer);
				renderPass.render(static_cast<uint32_t>(imageId), commandBuffer);
				frameBuffer.unbind(commandBuffer);
			});

			VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
			submitInfo.pWaitDstStageMask = &waitStages;

			if (i == 0)
			{
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores    = &imageAvailable;
			}

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers    = &m_commandPools[imageId][i];

			if (isPresentationPass)
			{
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores    = &renderComplete;
			}

			const VkQueue currentQueue = renderPass.m_queueType == vzt::QueueType::Graphic
			                                 ? m_device->getGraphicsQueue()
			                                 : throw std::runtime_error("Compute pipeline is not currently supported");

			graphicSubmissions.emplace_back(std::move(submitInfo));
		}

		vkResetFences(m_device->vkHandle(), 1, &inFlightFence);
		if (vkQueueSubmit(m_device->getGraphicsQueue(), static_cast<uint32_t>(graphicSubmissions.size()),
		                  graphicSubmissions.data(), inFlightFence) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit command buffers!");
		}
	}

	const Attachment& RenderGraph::getAttachment(const std::size_t imageId, const vzt::AttachmentHandle& handle) const
	{
		auto it = m_attachmentsIndices[imageId].find(handle);
		if (it != m_attachmentsIndices[imageId].end())
			return m_attachments[it->second];
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
