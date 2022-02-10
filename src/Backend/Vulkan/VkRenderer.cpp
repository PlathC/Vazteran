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
		const uint32_t imageCount           = m_swapChain.GetImageCount();
		const auto     swapChainImageFormat = m_swapChain.GetImageFormat();
		const auto     swapChainSize        = m_swapChain.GetFrameBufferSize();

		vzt::Program fsBlinnPhongProgram = vzt::Program(&m_device);

		auto vtxShader  = vzt::Shader("./shaders/fs_triangle.vert.spv", ShaderStage::VertexShader);
		auto fragShader = vzt::Shader("./shaders/blinn_phong.frag.spv", ShaderStage::FragmentShader);
		fsBlinnPhongProgram.SetShader(std::move(vtxShader));
		fsBlinnPhongProgram.SetShader(std::move(fragShader));
		fsBlinnPhongProgram.Compile();

		// Position
		m_fsDescriptorSetLayout.AddBinding(vzt::ShaderStage::FragmentShader, 0, vzt::DescriptorType::CombinedSampler);
		// Normals
		m_fsDescriptorSetLayout.AddBinding(vzt::ShaderStage::FragmentShader, 1, vzt::DescriptorType::CombinedSampler);
		// Albedo
		m_fsDescriptorSetLayout.AddBinding(vzt::ShaderStage::FragmentShader, 2, vzt::DescriptorType::CombinedSampler);

		m_fsDescriptorPool = vzt::DescriptorPool(&m_device, {vzt::DescriptorType::CombinedSampler});
		m_fsDescriptorPool.Allocate(imageCount, m_fsDescriptorSetLayout);

		m_compositionPipeline =
		    std::make_unique<vzt::GraphicPipeline>(&m_device, std::move(fsBlinnPhongProgram), m_fsDescriptorSetLayout);
		m_meshView = std::make_unique<vzt::MeshView>(&m_device, imageCount);

		m_compositionPipeline->RasterOptions().cullMode = vzt::CullMode::Front;

		m_offscreenCommandPool.AllocateCommandBuffers(imageCount);
		m_fsCommandPool.AllocateCommandBuffers(imageCount);

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

		BuildRenderingSupports();
	}

	Renderer::Renderer(Renderer&& other)
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

	Renderer& Renderer::operator=(Renderer&& other)
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

	void Renderer::SetScene(vzt::Scene* scene)
	{
		auto uiData = scene->CSceneUi();
		if (uiData.has_value())
		{
			m_ui = std::make_unique<vzt::VkUiRenderer>(m_instance, &m_device, m_window, m_swapChain.GetImageCount(),
			                                           m_frames[0].RenderPass(), uiData.value());
		}

		auto sceneModels = scene->CModels();
		for (const auto* model : sceneModels)
		{
			m_meshView->AddModel(model);
		}
	}

	void Renderer::Draw(const vzt::Camera& camera)
	{
		m_meshView->Update(camera);

		const bool recreate = m_swapChain.RenderFrame(
		    [&](uint32_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete, VkFence inFlightFence) {
			    m_offscreenCommandPool.RecordBuffer(imageId, [&](VkCommandBuffer commandBuffer) {
				    m_offscreenFrames[imageId].Bind(commandBuffer);
				    m_meshView->Record(imageId, m_offscreenFrames[imageId].RenderPass(), commandBuffer);

				    m_offscreenFrames[imageId].Unbind(commandBuffer);
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

			    if (vkQueueSubmit(m_device.GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
			    {
				    throw std::runtime_error("Failed to submit command buffer!");
			    }

			    m_fsCommandPool.RecordBuffer(imageId, [&](VkCommandBuffer commandBuffer) {
				    m_frames[imageId].Bind(commandBuffer);

				    m_fsDescriptorPool.Bind(imageId, commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				                            m_compositionPipeline->Layout());
				    m_compositionPipeline->Bind(commandBuffer, m_frames[imageId].RenderPass());

				    // Final composition as full screen quad
				    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

				    if (m_ui)
				    {
					    m_ui->Record(commandBuffer);
				    }

				    m_frames[imageId].Unbind(commandBuffer);
			    });

			    submitInfo.waitSemaphoreCount = 1;
			    submitInfo.pWaitSemaphores    = &m_offscreenSemaphores[imageId];
			    submitInfo.pWaitDstStageMask  = waitStages;
			    submitInfo.commandBufferCount = 1;
			    submitInfo.pCommandBuffers    = &m_fsCommandPool[imageId];

			    submitInfo.signalSemaphoreCount = 1;
			    submitInfo.pSignalSemaphores    = &renderComplete;

			    vkResetFences(m_device.VkHandle(), 1, &inFlightFence);
			    if (vkQueueSubmit(m_device.GraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS)
			    {
				    throw std::runtime_error("Failed to submit command buffer!");
			    }
		    });

		if (recreate)
		{
			vkDeviceWaitIdle(m_device.VkHandle());
		}
	}

	void Renderer::FrameBufferResized(vzt::Size2D<uint32_t> newSize)
	{
		m_swapChain.SetFrameBufferSize(std::move(newSize));
		m_swapChain.Recreate(m_surface);

		BuildRenderingSupports();
	}

	void Renderer::BuildRenderingSupports()
	{
		const uint32_t imageCount           = m_swapChain.GetImageCount();
		const auto     swapChainImageFormat = m_swapChain.GetImageFormat();
		const auto     swapChainSize        = m_swapChain.GetFrameBufferSize();

		vzt::Format depthFormat     = m_device.ChosenPhysicalDevice()->FindDepthFormat();
		const auto  swapChainImages = m_swapChain.GetImagesKHR();

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

			offscreenAttachments[0].SetFinalLayout(vzt::ImageLayout::ShaderReadOnlyOptimal);
			offscreenAttachments[1].SetFinalLayout(vzt::ImageLayout::ShaderReadOnlyOptimal);
			offscreenAttachments[2].SetFinalLayout(vzt::ImageLayout::ShaderReadOnlyOptimal);
			offscreenAttachments[3].SetFinalLayout(vzt::ImageLayout::DepthStencilAttachmentOptimal);

			offscreenAttachmentsTexture.emplace_back(offscreenAttachments[0].AsTexture());
			offscreenAttachmentsTexture.emplace_back(offscreenAttachments[1].AsTexture());
			offscreenAttachmentsTexture.emplace_back(offscreenAttachments[2].AsTexture());

			IndexedUniform<vzt::Texture*> texturesDescriptors;
			texturesDescriptors[0] = offscreenAttachmentsTexture[0];
			texturesDescriptors[1] = offscreenAttachmentsTexture[1];
			texturesDescriptors[2] = offscreenAttachmentsTexture[2];

			m_fsDescriptorPool.Update(i, texturesDescriptors);

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

			screenAttachments[0].SetFinalLayout(vzt::ImageLayout::PresentSrcKHR);
			screenAttachments[1].SetFinalLayout(vzt::ImageLayout::DepthStencilAttachmentOptimal);

			dependencies = {{VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT,
			                 VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			                 VK_DEPENDENCY_BY_REGION_BIT}};

			m_frames.emplace_back(&m_device, 1, std::move(dependencies), std::move(screenAttachments), swapChainSize);
		}

		m_meshView->Configure({m_offscreenFrames[0].RenderPass(), swapChainImageFormat, swapChainSize});
		m_compositionPipeline->Configure({m_frames[0].RenderPass(), swapChainImageFormat, swapChainSize});
	}
} // namespace vzt
