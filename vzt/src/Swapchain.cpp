#include "vzt/Swapchain.hpp"

#include <cassert>

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
        assert(m_device->getPresentQueue() && "Device must have a present queue to use the swapchain");
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

    std::optional<Submission> Swapchain::getSubmission()
    {
        vkWaitForFences(m_device->getHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t       imageIndex;
        const VkResult result =
            vkAcquireNextImageKHR(m_device->getHandle(), m_handle, UINT64_MAX,
                                  m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            return {};
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            logger::error("Failed to acquire swapchain image!");

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(m_device->getHandle(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

        // Mark the image as now being in use by this frame
        m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

        Submission submission;
        submission.imageId        = imageIndex;
        submission.imageAvailable = m_imageAvailableSemaphores[m_currentFrame];
        submission.renderComplete = m_renderFinishedSemaphores[m_currentFrame];
        submission.frameComplete  = m_inFlightFences[m_currentFrame];
        return submission;
    }

    bool Swapchain::present(const Submission& submission)
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphores[m_currentFrame];

        const VkSwapchainKHR swapChains[] = {m_handle};
        presentInfo.swapchainCount        = 1;
        presentInfo.pSwapchains           = swapChains;
        presentInfo.pImageIndices         = &submission.imageId;

        const View<Queue> presentQueue = m_device->getPresentQueue();
        const VkResult    result       = vkQueuePresentKHR(presentQueue->getHandle(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            return true;
        }

        if (result != VK_SUCCESS)
            logger::error("Failed to present swap chain image!");

        m_currentFrame = (m_currentFrame + 1) % m_configuration.maxFramesInFlight;
        return false;
    }

    void Swapchain::create()
    {
        const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(m_surface->getFormats(m_device));
        const VkPresentModeKHR   presentMode   = chooseSwapPresentMode(m_surface->getPresentModes(m_device));

        const VkSurfaceCapabilitiesKHR capabilities = m_surface->getCapabilities(m_device);
        const VkExtent2D               extent       = chooseExtent(capabilities);

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

        auto queues = m_device->getQueues();
        if (!m_device->getPresentQueue())
            logger::error("Picked device does not have presentation capabilities.");

        std::vector<uint32_t> queueFamilyIndices{};
        queueFamilyIndices.reserve(queues.size());
        for (const auto& queue : queues)
            queueFamilyIndices.emplace_back(queue->getId());

        if (queueFamilyIndices.size() == 1)
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices   = nullptr;
        }
        else
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
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

        m_images.resize(m_imageNb);
        vkGetSwapchainImagesKHR(m_device->getHandle(), m_handle, &m_imageNb, m_images.data());
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

    VkExtent2D Swapchain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;
        return {m_extent.width, m_extent.height};
    }
} // namespace vzt