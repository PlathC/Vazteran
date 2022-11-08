#include "vzt/Swapchain.hpp"

#include <vulkan/vulkan_core.h>

#include "vzt/Core/Logger.hpp"
#include "vzt/Device.hpp"
#include "vzt/Image.hpp"
#include "vzt/Instance.hpp"
#include "vzt/Surface.hpp"

namespace vzt
{
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            const bool isSRGB = availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            if (isSRGB)
                return availableFormat;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    Swapchain::Swapchain(View<Device> device, View<Surface> surface, Extent2D extent,
                         SwapchainConfiguration configuration)
        : m_configuration(configuration), m_device(device), m_surface(surface), m_extent(extent)
    {
        create();
    }

    Swapchain::~Swapchain()
    {
        for (std::size_t i = 0; i < m_configuration.maxFramesInFlight; i++)
        {
            vkDestroySemaphore(m_device->getHandle(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_device->getHandle(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_device->getHandle(), m_inFlightFences[i], nullptr);
        }

        for (auto& imageInFlight : m_imagesInFlight)
            imageInFlight = VK_NULL_HANDLE;

        cleanup();
    }

    void Swapchain::create()
    {
        const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(m_surface->getFormats(m_device));
        const VkPresentModeKHR   presentMode   = chooseSwapPresentMode(m_surface->getPresentModes(m_device));

        const VkSurfaceCapabilitiesKHR capabilities = m_surface->getCapabilities(m_device);
        const VkExtent2D               extent       = chooseSwapExtent(capabilities);

        m_imageNb = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && m_imageNb > capabilities.maxImageCount)
            m_imageNb = capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = m_surface->getHandle();
        createInfo.minImageCount    = m_imageNb;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = toVulkan(ImageUsage::ColorAttachment);

        PhysicalDevice physicalDevice = m_device->getHardware();
        auto           presentId      = physicalDevice.getPresentQueueFamilyIndex(m_surface);
        if (!presentId)
            logger::error("Picked device does not have presentation capabilities.");

        View<Queue> graphicsQueue = m_device->getQueue(QueueType::Graphics);

        const uint32_t graphicsQueueId      = graphicsQueue->getId();
        const uint32_t queueFamilyIndices[] = {graphicsQueueId, *presentId};
        if (graphicsQueue->getId() == *presentId)
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices   = nullptr;
        }
        else
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }

        createInfo.preTransform   = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;
        createInfo.oldSwapchain   = VK_NULL_HANDLE;

        vkCheck(vkCreateSwapchainKHR(m_device->getHandle(), &createInfo, nullptr, &m_handle),
                "Failed to create swapchain");

        m_imageAvailableSemaphores.resize(m_configuration.maxFramesInFlight);
        m_renderFinishedSemaphores.resize(m_configuration.maxFramesInFlight);
        m_inFlightFences.resize(m_configuration.maxFramesInFlight);
        m_imagesInFlight.resize(m_imageNb, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (std::size_t i = 0; i < m_configuration.maxFramesInFlight; i++)
        {
            vkCheck(vkCreateSemaphore(m_device->getHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]),
                    "Failed to create synchronization objects for a frame");
            vkCheck(vkCreateSemaphore(m_device->getHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]),
                    "Failed to create synchronization objects for a frame");
            vkCheck(vkCreateFence(m_device->getHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]),
                    "Failed to create synchronization objects for a frame");
        }
    }

    void Swapchain::cleanup()
    {
        for (auto& fence : m_imagesInFlight)
        {
            if (fence != VK_NULL_HANDLE)
                vkWaitForFences(m_device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
        }

        vkDestroySwapchainKHR(m_device->getHandle(), m_handle, nullptr);
    }

    VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;
        return {m_extent.width, m_extent.height};
    }
} // namespace vzt
