#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Backend/Vulkan/SwapChain.hpp"

namespace vzt
{
	SwapChain::SwapChain(vzt::Device* device, VkSurfaceKHR surface, vzt::Size2D<uint32_t> swapChainSize)
	    : m_surface(surface), m_device(device), m_swapChainSize(swapChainSize)
	{
		createSwapChain();

		createSynchronizationObjects();
	}

	bool SwapChain::render(const SubmitFunction submitFunction)
	{
		vkWaitForFences(m_device->vkHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result =
		    vkAcquireNextImageKHR(m_device->vkHandle(), m_vkHandle, UINT64_MAX,
		                          m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			return true;
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to acquire swapchain image!");

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(m_device->vkHandle(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

		// Mark the image as now being in use by this frame
		m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

		submitFunction(imageIndex, m_imageAvailableSemaphores[m_currentFrame],
		               m_renderFinishedSemaphores[m_currentFrame], m_inFlightFences[m_currentFrame]);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphores[m_currentFrame];

		VkSwapchainKHR swapChains[] = {m_vkHandle};
		presentInfo.swapchainCount  = 1;
		presentInfo.pSwapchains     = swapChains;
		presentInfo.pImageIndices   = &imageIndex;

		result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);
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

	void SwapChain::setFrameBufferSize(vzt::Size2D<uint32_t> newSize)
	{
		m_swapChainSize      = std::move(newSize);
		m_framebufferResized = true;
	};

	void SwapChain::recreate(VkSurfaceKHR surface)
	{
		cleanup();

		m_surface = surface;

		createSwapChain();
	}

	SwapChain::~SwapChain()
	{
		for (std::size_t i = 0; i < MaxFramesInFlight; i++)
		{
			vkDestroySemaphore(m_device->vkHandle(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device->vkHandle(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device->vkHandle(), m_inFlightFences[i], nullptr);
		}

		for (auto& imageInFlight : m_imagesInFlight)
			imageInFlight = VK_NULL_HANDLE;

		cleanup();
	}

	void SwapChain::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = m_device->getPhysicalDevice()->querySwapChainSupport(m_surface);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR   m_presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D         extent        = chooseSwapExtent(swapChainSupport.capabilities);

		m_swapChainImageFormat = static_cast<vzt::Format>(surfaceFormat.format);

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
		createInfo.imageUsage = static_cast<VkImageUsageFlags>(vzt::toUnderlying(vzt::ImageUsage::ColorAttachment));

		QueueFamilyIndices indices              = m_device->getDeviceQueueFamilyIndices();
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

		if (vkCreateSwapchainKHR(m_device->vkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swapchain!");
		}
	}

	std::vector<VkImage> SwapChain::getRenderImages()
	{
		auto swapChainImages = std::vector<VkImage>(m_imageCount);
		vkGetSwapchainImagesKHR(m_device->vkHandle(), m_vkHandle, &m_imageCount, swapChainImages.data());

		return swapChainImages;
	}

	void SwapChain::createSynchronizationObjects()
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
			if (vkCreateSemaphore(m_device->vkHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) !=
			        VK_SUCCESS ||
			    vkCreateSemaphore(m_device->vkHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) !=
			        VK_SUCCESS ||
			    vkCreateFence(m_device->vkHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create synchronization objects for a frame!");
			}
		}
	}

	void SwapChain::cleanup()
	{
		for (std::size_t i = 0; i < m_imagesInFlight.size(); i++)
		{
			if (m_imagesInFlight[i] != VK_NULL_HANDLE)
			{
				vkWaitForFences(m_device->vkHandle(), 1, &m_imagesInFlight[i], VK_TRUE, UINT64_MAX);
			}
		}

		vkDestroySwapchainKHR(m_device->vkHandle(), m_vkHandle, nullptr);
	}

	VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

	VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

	VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			return {static_cast<uint32_t>(m_swapChainSize.width), static_cast<uint32_t>(m_swapChainSize.height)};
		}
	}
} // namespace vzt
