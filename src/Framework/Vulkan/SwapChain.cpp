#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Vazteran/Framework/Vulkan/Device.hpp"
#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Framework/Vulkan/RenderPass.hpp"
#include "Vazteran/Framework/Vulkan/SwapChain.hpp"

namespace vzt
{
	SwapChain::SwapChain(vzt::Device* device, VkSurfaceKHR surface, vzt::Size2D<uint32_t> swapChainSize)
	    : m_surface(surface), m_device(device),
	      m_commandPool(std::make_unique<vzt::CommandPool>(m_device, MaxFramesInFlight)), m_swapChainSize(swapChainSize)
	{
		CreateSwapChain();
		CreateDepthResources();
		CreateRenderSupport();

		CreateSynchronizationObjects();
	}

	bool SwapChain::DrawFrame()
	{
		if (vkWaitForFences(m_device->VkHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("Failed to wait for fence " + std::to_string(m_currentFrame) + "!");
		}

		uint32_t imageIndex;
		VkResult result =
		    vkAcquireNextImageKHR(m_device->VkHandle(), m_vkHandle, UINT64_MAX,
		                          m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			return true;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swapchain image!");
		}

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_device->VkHandle(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		// Mark the image as now being in use by this frame
		m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore          waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
		VkPipelineStageFlags waitStages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount         = 1;
		submitInfo.pWaitSemaphores            = waitSemaphores;
		submitInfo.pWaitDstStageMask          = waitStages;
		submitInfo.commandBufferCount         = 1;
		submitInfo.pCommandBuffers            = &(*m_commandPool)[imageIndex];

		VkSemaphore signalSemaphores[]  = {m_renderFinishedSemaphores[m_currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = signalSemaphores;

		vkResetFences(m_device->VkHandle(), 1, &m_inFlightFences[m_currentFrame]);

		if (vkQueueSubmit(m_device->GraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores    = signalSemaphores;

		VkSwapchainKHR swapChains[] = {m_vkHandle};
		presentInfo.swapchainCount  = 1;
		presentInfo.pSwapchains     = swapChains;
		presentInfo.pImageIndices   = &imageIndex;
		presentInfo.pResults        = nullptr; // Optional

		result = vkQueuePresentKHR(m_device->PresentQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			return true;
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}

		m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;
		return false;
	}

	void SwapChain::SetFrameBufferSize(vzt::Size2D<uint32_t> newSize)
	{
		m_frameBufferSize    = newSize;
		m_framebufferResized = true;
	};

	void SwapChain::Record(RenderFunction renderFunction)
	{
		for (std::size_t i = 0; i < m_imageCount; i++)
		{
			m_commandPool->RecordBuffer(i, [&](VkCommandBuffer commandBuffer) {
				m_renderPass->Bind(commandBuffer, &m_frames[i]);
				renderFunction(i, commandBuffer, m_renderPass.get());
				m_renderPass->Unbind(commandBuffer);
			});
		}
	}

	void SwapChain::Recreate(VkSurfaceKHR surface)
	{
		Cleanup();

		m_surface = surface;

		CreateSwapChain();
		CreateDepthResources();
		CreateRenderSupport();
	}

	SwapChain::~SwapChain()
	{
		for (std::size_t i = 0; i < MaxFramesInFlight; i++)
		{
			vkDestroySemaphore(m_device->VkHandle(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device->VkHandle(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device->VkHandle(), m_inFlightFences[i], nullptr);
		}

		for (auto& imageInFlight : m_imagesInFlight)
			imageInFlight = VK_NULL_HANDLE;

		Cleanup();
	}

	void SwapChain::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = m_device->ChosenPhysicalDevice()->QuerySwapChainSupport(m_surface);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR   m_presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D         extent        = ChooseSwapExtent(swapChainSupport.capabilities);

		m_imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
		    m_imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			m_imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface          = m_surface;
		createInfo.minImageCount    = m_imageCount;
		createInfo.imageFormat      = surfaceFormat.format;
		createInfo.imageColorSpace  = surfaceFormat.colorSpace;
		createInfo.imageExtent      = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		m_swapChainImageFormat      = surfaceFormat.format;
		m_frameBufferSize           = {extent.width, extent.height};

		QueueFamilyIndices indices              = m_device->DeviceQueueFamilyIndices();
		uint32_t           queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices   = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;       // Optional
			createInfo.pQueueFamilyIndices   = nullptr; // Optional
		}

		createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode    = m_presentMode;
		createInfo.clipped        = VK_TRUE;
		createInfo.oldSwapchain   = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_device->VkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swapchain!");
		}
	}

	void SwapChain::CreateDepthResources()
	{
		VkFormat depthFormat = m_device->ChosenPhysicalDevice()->FindDepthFormat();
		m_depthImageData     = {vzt::ImageView(m_device, m_swapChainSize, depthFormat,
		                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
		                                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
                            Sampler(m_device)};
	}

	void SwapChain::CreateRenderSupport()
	{
		m_renderPass         = std::make_unique<vzt::RenderPass>(m_device, m_swapChainSize, m_swapChainImageFormat);
		auto swapChainImages = std::vector<VkImage>(m_imageCount);
		vkGetSwapchainImagesKHR(m_device->VkHandle(), m_vkHandle, &m_imageCount, swapChainImages.data());

		m_frames.reserve(m_imageCount);
		m_commandPool->AllocateCommandBuffers(m_imageCount);
		for (std::size_t i = 0; i < m_imageCount; i++)
		{
			m_frames.emplace_back(m_device, m_renderPass.get(), swapChainImages[i], &m_depthImageData.value().first,
			                      m_swapChainImageFormat, m_swapChainSize);
		}
	}

	void SwapChain::CreateSynchronizationObjects()
	{
		m_imageAvailableSemaphores.resize(MaxFramesInFlight);
		m_renderFinishedSemaphores.resize(MaxFramesInFlight);
		m_inFlightFences.resize(MaxFramesInFlight);
		m_imagesInFlight.resize(m_imageCount, VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (std::size_t i = 0; i < MaxFramesInFlight; i++)
		{
			if (vkCreateSemaphore(m_device->VkHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) !=
			        VK_SUCCESS ||
			    vkCreateSemaphore(m_device->VkHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) !=
			        VK_SUCCESS ||
			    vkCreateFence(m_device->VkHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create synchronization objects for a frame!");
			}
		}
	}

	void SwapChain::Cleanup()
	{
		for (std::size_t i = 0; i < m_frames.size(); i++)
		{
			if (m_imagesInFlight[i] != VK_NULL_HANDLE)
			{
				vkWaitForFences(m_device->VkHandle(), 1, &m_imagesInFlight[i], VK_TRUE, UINT64_MAX);
			}
		}

		m_frames.clear();
		m_depthImageData.reset();

		vkDestroySwapchainKHR(m_device->VkHandle(), m_vkHandle, nullptr);
	}

	VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			return {static_cast<uint32_t>(m_frameBufferSize.width), static_cast<uint32_t>(m_frameBufferSize.height)};
		}
	}
} // namespace vzt
