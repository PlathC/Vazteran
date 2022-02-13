#include "Vazteran/Backend/Vulkan/VkRenderer.hpp"
#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/VkUiRenderer.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"

namespace vzt
{
	Renderer::Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size)
	    : m_surface(surface), m_device(instance, m_surface), m_offscreenCommandPool(&m_device),
	      m_fsCommandPool(&m_device), m_swapChain(&m_device, surface, size), m_instance(instance), m_window(window),
	      m_fsDescriptorSetLayout(&m_device)
	{
		const uint32_t imageCount           = m_swapChain.imageCount();
		const auto     swapChainImageFormat = m_swapChain.imageFormat();
		const auto     swapChainSize        = m_swapChain.frameBufferSize();

		vzt::Program fsBlinnPhongProgram = vzt::Program(&m_device);

		auto vtxShader  = vzt::Shader("./shaders/fs_triangle.vert.spv", ShaderStage::VertexShader);
		auto fragShader = vzt::Shader("./shaders/blinn_phong.frag.spv", ShaderStage::FragmentShader);
		fsBlinnPhongProgram.setShader(std::move(vtxShader));
		fsBlinnPhongProgram.setShader(std::move(fragShader));
		fsBlinnPhongProgram.compile();

		// Position
		m_fsDescriptorSetLayout.addBinding(vzt::ShaderStage::FragmentShader, 0, vzt::DescriptorType::CombinedSampler);
		// Normals
		m_fsDescriptorSetLayout.addBinding(vzt::ShaderStage::FragmentShader, 1, vzt::DescriptorType::CombinedSampler);
		// Albedo
		m_fsDescriptorSetLayout.addBinding(vzt::ShaderStage::FragmentShader, 2, vzt::DescriptorType::CombinedSampler);

		m_fsDescriptorPool = vzt::DescriptorPool(&m_device, {vzt::DescriptorType::CombinedSampler});
		m_fsDescriptorPool.allocate(imageCount, m_fsDescriptorSetLayout);

		m_compositionPipeline =
		    std::make_unique<vzt::GraphicPipeline>(&m_device, std::move(fsBlinnPhongProgram), m_fsDescriptorSetLayout);
		m_meshView = std::make_unique<vzt::MeshView>(&m_device, imageCount);

		m_compositionPipeline->getRasterOptions().cullMode = vzt::CullMode::Front;

		m_offscreenCommandPool.allocateCommandBuffers(imageCount);
		m_fsCommandPool.allocateCommandBuffers(imageCount);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		m_offscreenSemaphores.resize(imageCount);
		for (auto& semaphore : m_offscreenSemaphores)
		{
			if (vkCreateSemaphore(m_device.VkHandle(), &semaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS)
			{
				throw std::runtime_error("Can't create offscreen semaphore");
			}
		}

		buildRenderingSupports();
	}

	Renderer::Renderer(Renderer&& other) noexcept
	    : m_device(std::move(other.m_device)), m_swapChain(std::move(other.m_swapChain)),
	      m_offscreenCommandPool(std::move(other.m_offscreenCommandPool)),
	      m_fsCommandPool(std::move(other.m_fsCommandPool)),
	      m_fsDescriptorSetLayout(std::move(other.m_fsDescriptorSetLayout))

	{
		std::swap(m_surface, other.m_surface);

		std::swap(m_meshView, other.m_meshView);
		std::swap(m_ui, other.m_ui);

		std::swap(m_offscreenSemaphores, other.m_offscreenSemaphores);
		std::swap(m_offscreenFrames, other.m_offscreenFrames);
		std::swap(m_frames, other.m_frames);

		std::swap(m_fsDescriptorPool, other.m_fsDescriptorPool);
		std::swap(m_compositionPipeline, other.m_compositionPipeline);
		std::swap(m_samplersDescriptors, other.m_samplersDescriptors);

		std::swap(m_instance, other.m_instance);
		std::swap(m_window, other.m_window);
	}

	Renderer& Renderer::operator=(Renderer&& other) noexcept
	{
		std::swap(m_surface, other.m_surface);

		std::swap(m_device, other.m_device);
		std::swap(m_swapChain, other.m_swapChain);

		std::swap(m_meshView, other.m_meshView);
		std::swap(m_ui, other.m_ui);

		std::swap(m_offscreenSemaphores, other.m_offscreenSemaphores);
		std::swap(m_offscreenCommandPool, other.m_offscreenCommandPool);
		std::swap(m_offscreenFrames, other.m_offscreenFrames);
		std::swap(m_frames, other.m_frames);

		std::swap(m_fsDescriptorSetLayout, other.m_fsDescriptorSetLayout);
		std::swap(m_fsDescriptorPool, other.m_fsDescriptorPool);
		std::swap(m_fsCommandPool, other.m_fsCommandPool);

		std::swap(m_compositionPipeline, other.m_compositionPipeline);
		std::swap(m_samplersDescriptors, other.m_samplersDescriptors);

		std::swap(m_instance, other.m_instance);
		std::swap(m_window, other.m_window);

		return *this;
	}

	Renderer::~Renderer()
	{
		for (auto& semaphore : m_offscreenSemaphores)
		{
			vkDestroySemaphore(m_device.VkHandle(), semaphore, nullptr);
		}
		m_offscreenSemaphores = {};
	}

	void Renderer::setScene(vzt::Scene* scene)
	{
		auto uiData = scene->cSceneUi();
		if (uiData.has_value())
		{
			m_ui = std::make_unique<vzt::VkUiRenderer>(m_instance, &m_device, m_window, m_swapChain.imageCount(),
			                                           m_frames[0].getRenderPass(), uiData.value());
		}

		auto sceneModels = scene->cModels();
		for (const auto* model : sceneModels)
		{
			m_meshView->addModel(model);
		}
	}

	void Renderer::draw(const vzt::Camera& camera)
	{
		m_meshView->update(camera);

		const bool recreate = m_swapChain.render(
		    [&](uint32_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete, VkFence inFlightFence) {
			    m_offscreenCommandPool.recordBuffer(imageId, [&](VkCommandBuffer commandBuffer) {
				    m_offscreenFrames[imageId].bind(commandBuffer);
				    m_meshView->record(imageId, m_offscreenFrames[imageId].getRenderPass(), commandBuffer);

				    m_offscreenFrames[imageId].unbind(commandBuffer);
			    });

			    VkSubmitInfo submitInfo{};
			    submitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

			    submitInfo.waitSemaphoreCount = 1;
			    submitInfo.pWaitSemaphores    = &imageAvailable;
			    submitInfo.pWaitDstStageMask  = waitStages;
			    submitInfo.commandBufferCount = 1;
			    submitInfo.pCommandBuffers    = &m_offscreenCommandPool[imageId];

			    submitInfo.signalSemaphoreCount = 1;
			    submitInfo.pSignalSemaphores    = &m_offscreenSemaphores[imageId];

			    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
			    {
				    throw std::runtime_error("Failed to submit command buffer!");
			    }

			    m_fsCommandPool.recordBuffer(imageId, [&](VkCommandBuffer commandBuffer) {
				    m_frames[imageId].bind(commandBuffer);

				    m_fsDescriptorPool.bind(imageId, commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				                            m_compositionPipeline->layout());
				    m_compositionPipeline->bind(commandBuffer, m_frames[imageId].getRenderPass());

				    // Final composition as full screen quad
				    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

				    if (m_ui)
				    {
					    m_ui->record(commandBuffer);
				    }

				    m_frames[imageId].unbind(commandBuffer);
			    });

			    submitInfo.waitSemaphoreCount = 1;
			    submitInfo.pWaitSemaphores    = &m_offscreenSemaphores[imageId];
			    submitInfo.pWaitDstStageMask  = waitStages;
			    submitInfo.commandBufferCount = 1;
			    submitInfo.pCommandBuffers    = &m_fsCommandPool[imageId];

			    submitInfo.signalSemaphoreCount = 1;
			    submitInfo.pSignalSemaphores    = &renderComplete;

			    vkResetFences(m_device.VkHandle(), 1, &inFlightFence);
			    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS)
			    {
				    throw std::runtime_error("Failed to submit command buffer!");
			    }
		    });

		if (recreate)
		{
			vkDeviceWaitIdle(m_device.VkHandle());
		}
	}

	void Renderer::resize(vzt::Size2D<uint32_t> newSize)
	{
		m_swapChain.setFrameBufferSize(std::move(newSize));
		m_swapChain.recreate(m_surface);

		buildRenderingSupports();
	}

	void Renderer::buildRenderingSupports()
	{
		const uint32_t imageCount           = m_swapChain.imageCount();
		const auto     swapChainImageFormat = m_swapChain.imageFormat();
		const auto     swapChainSize        = m_swapChain.frameBufferSize();

		vzt::Format depthFormat     = m_device.getPhysicalDevice()->findDepthFormat();
		const auto  swapChainImages = m_swapChain.getImagesKHR();

		m_frames.clear();
		m_offscreenFrames.clear();
		m_frames.reserve(imageCount);
		m_offscreenFrames.reserve(imageCount);

		std::vector<vzt::Texture*> offscreenAttachmentsTexture;
		offscreenAttachmentsTexture.reserve(3 * imageCount);
		for (std::size_t i = 0; i < imageCount; i++)
		{
			std::vector<vzt::Attachment> offscreenAttachments;
			offscreenAttachments.emplace_back(&m_device, swapChainSize, vzt::Format::R16G16B16A16SFloat,
			                                  vzt::ImageUsage::ColorAttachment);
			offscreenAttachments.emplace_back(&m_device, swapChainSize, vzt::Format::R16G16B16A16SFloat,
			                                  vzt::ImageUsage::ColorAttachment);
			offscreenAttachments.emplace_back(&m_device, swapChainSize, vzt::Format::R8G8B8A8UNorm,
			                                  vzt::ImageUsage::ColorAttachment);

			offscreenAttachments.emplace_back(&m_device, swapChainSize, depthFormat,
			                                  vzt::ImageUsage::DepthStencilAttachment);

			offscreenAttachments[0].setFinalLayout(vzt::ImageLayout::ShaderReadOnlyOptimal);
			offscreenAttachments[1].setFinalLayout(vzt::ImageLayout::ShaderReadOnlyOptimal);
			offscreenAttachments[2].setFinalLayout(vzt::ImageLayout::ShaderReadOnlyOptimal);
			offscreenAttachments[3].setFinalLayout(vzt::ImageLayout::DepthStencilAttachmentOptimal);

			offscreenAttachmentsTexture.emplace_back(offscreenAttachments[0].asTexture());
			offscreenAttachmentsTexture.emplace_back(offscreenAttachments[1].asTexture());
			offscreenAttachmentsTexture.emplace_back(offscreenAttachments[2].asTexture());

			IndexedUniform<vzt::Texture*> texturesDescriptors;
			texturesDescriptors[0] = offscreenAttachmentsTexture[0];
			texturesDescriptors[1] = offscreenAttachmentsTexture[1];
			texturesDescriptors[2] = offscreenAttachmentsTexture[2];

			m_fsDescriptorPool.update(i, texturesDescriptors);

			std::vector<VkSubpassDependency> dependencies = {
			    {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT,
			     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			     VK_DEPENDENCY_BY_REGION_BIT},
			    {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
			     VK_DEPENDENCY_BY_REGION_BIT}};

			m_offscreenFrames.emplace_back(&m_device, 1, std::move(dependencies), std::move(offscreenAttachments),
			                               swapChainSize);

			std::vector<vzt::Attachment> screenAttachments;
			screenAttachments.emplace_back(&m_device, swapChainImages[i], swapChainImageFormat,
			                               vzt::ImageLayout::ColorAttachmentOptimal, vzt::ImageAspect::Color);
			screenAttachments.emplace_back(&m_device, swapChainSize, depthFormat,
			                               vzt::ImageUsage::DepthStencilAttachment);

			screenAttachments[0].setFinalLayout(vzt::ImageLayout::PresentSrcKHR);
			screenAttachments[1].setFinalLayout(vzt::ImageLayout::DepthStencilAttachmentOptimal);

			dependencies = {{VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT,
			                 VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			                 VK_DEPENDENCY_BY_REGION_BIT}};

			m_frames.emplace_back(&m_device, 1, std::move(dependencies), std::move(screenAttachments), swapChainSize);
		}

		m_meshView->configure({m_offscreenFrames[0].getRenderPass(), swapChainImageFormat, swapChainSize});
		m_compositionPipeline->configure({m_frames[0].getRenderPass(), swapChainImageFormat, swapChainSize});
	}
} // namespace vzt
