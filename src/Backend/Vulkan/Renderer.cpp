#include "Vazteran/Backend/Vulkan/Renderer.hpp"
#include "Vazteran/Backend/Vulkan/UiRenderer.hpp"
#include "Vazteran/Views/MeshView.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	Renderer::Renderer(Window& window)
	    : m_surface(window.getSurface()), m_device(window.getInstance(), m_surface),
	      m_swapChain(&m_device, m_surface, window.getFrameBufferSize()), m_commandPool(&m_device)
	{
		const uint32_t             imageCount      = m_swapChain.getImageCount();
		const std::vector<VkImage> swapChainImages = m_swapChain.getRenderImages();
		m_commandPool.allocateCommandBuffers(imageCount);
		for (uint32_t i = 0; i < imageCount; i++)
		{
			m_commandPool.recordBuffer(i, [&](VkCommandBuffer commandBuffer) {
				VkImageMemoryBarrier transition{};
				transition.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				transition.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
				transition.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				transition.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
				transition.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
				transition.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
				transition.subresourceRange.baseMipLevel   = 0;
				transition.subresourceRange.levelCount     = 1;
				transition.subresourceRange.baseArrayLayer = 0;
				transition.subresourceRange.layerCount     = 1;

				transition.image = swapChainImages[i];
				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				                     0, 0, nullptr, 0, nullptr, 1, &transition);
			});
		}

		m_connectionHolder.subscribe<Window::FrameBufferResize, &Renderer::resize>(window, *this);
	}

	Renderer::Renderer(Renderer&& other) noexcept
	    : m_device(std::move(other.m_device)), m_swapChain(std::move(other.m_swapChain)),
	      m_commandPool(std::move(other.m_commandPool))
	{
		std::swap(m_surface, other.m_surface);
		std::swap(m_renderGraph, other.m_renderGraph);
	}

	Renderer& Renderer::operator=(Renderer&& other) noexcept
	{
		std::swap(m_surface, other.m_surface);

		std::swap(m_device, other.m_device);
		std::swap(m_swapChain, other.m_swapChain);
		std::swap(m_commandPool, other.m_commandPool);
		std::swap(m_renderGraph, other.m_renderGraph);

		return *this;
	}

	Renderer::~Renderer() {}

	void Renderer::setRenderGraph(RenderGraph renderGraph)
	{
		m_renderGraph = std::move(renderGraph);
		refresh();
	}

	void Renderer::render()
	{
		const bool recreate = m_swapChain.render(
		    [&](uint32_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete, VkFence inFlightFence) {
			    if (m_renderGraph)
			    {
				    m_renderGraph->render(imageId, imageAvailable, renderComplete, inFlightFence);
			    }
			    else
			    {
				    VkSubmitInfo emptySubmit{};
				    emptySubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				    constexpr VkPipelineStageFlags waitStage =
				        VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				    emptySubmit.pWaitDstStageMask    = &waitStage;
				    emptySubmit.waitSemaphoreCount   = 1;
				    emptySubmit.pWaitSemaphores      = &imageAvailable;
				    emptySubmit.signalSemaphoreCount = 1;
				    emptySubmit.pSignalSemaphores    = &renderComplete;
				    emptySubmit.commandBufferCount   = 1;
				    emptySubmit.pCommandBuffers      = &m_commandPool[imageId];

				    vkResetFences(m_device.vkHandle(), 1, &inFlightFence);
				    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &emptySubmit, inFlightFence) != VK_SUCCESS)
					    throw std::runtime_error("Can't submit to graphics queue.");
			    }
		    });

		if (recreate)
		{
			vkDeviceWaitIdle(m_device.vkHandle());
		}
	}

	void Renderer::resize(Window::FrameBufferResize newSize)
	{
		m_swapChain.setFrameBufferSize(std::move(newSize.size));
		m_swapChain.recreate(m_surface);

		refresh();
	}

	void Renderer::refresh()
	{
		if (!m_renderGraph)
			return;

		m_renderGraph->configure(&m_device, m_swapChain.getRenderImages(), m_swapChain.getFrameBufferSize(),
		                         m_swapChain.getImageFormat(), m_device.getPhysicalDevice()->findDepthFormat());
	}

	void Renderer::synchronize() { vkDeviceWaitIdle(m_device.vkHandle()); }

	uint32_t Renderer::getImageCount() const { return m_swapChain.getImageCount(); }

} // namespace vzt
