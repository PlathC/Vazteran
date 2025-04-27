#include "vzt/Vulkan/Swapchain.hpp"

#include "vzt/Core/Logger.hpp"
#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/Image.hpp"
#include "vzt/Vulkan/Surface.hpp"

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

    Swapchain::Swapchain(View<Device> device, View<Surface> surface, SwapchainBuilder configuration)
        : DeviceObject<VkSwapchainKHR>(device), m_configuration(configuration), m_surface(surface)
    {
        assert(m_device->getPresentQueue() && "Device must have a present queue to use the swapchain");

        m_imageAvailableSemaphores.resize(m_configuration.maxFramesInFlight, VK_NULL_HANDLE);
        m_renderFinishedSemaphores.resize(m_configuration.maxFramesInFlight, VK_NULL_HANDLE);
        m_inFlightFences.resize(m_configuration.maxFramesInFlight, VK_NULL_HANDLE);
        m_imagesInFlight.resize(m_configuration.maxFramesInFlight, VK_NULL_HANDLE);

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

        create();
    }

    Swapchain::Swapchain(Swapchain&& other) noexcept : DeviceObject<VkSwapchainKHR>(std::move(other))
    {
        std::swap(m_configuration, other.m_configuration);
        std::swap(m_surface, other.m_surface);
        std::swap(m_extent, other.m_extent);
        std::swap(m_framebufferResized, other.m_framebufferResized);
        std::swap(m_currentFrame, other.m_currentFrame);
        std::swap(m_currentImage, other.m_currentImage);
        std::swap(m_imageNb, other.m_imageNb);
        std::swap(m_images, other.m_images);
        std::swap(m_userImages, other.m_userImages);
        std::swap(m_imageAvailableSemaphores, other.m_imageAvailableSemaphores);
        std::swap(m_renderFinishedSemaphores, other.m_renderFinishedSemaphores);
        std::swap(m_inFlightFences, other.m_inFlightFences);
        std::swap(m_imagesInFlight, other.m_imagesInFlight);
    }

    Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
    {
        std::swap(m_configuration, other.m_configuration);
        std::swap(m_surface, other.m_surface);
        std::swap(m_extent, other.m_extent);
        std::swap(m_framebufferResized, other.m_framebufferResized);
        std::swap(m_currentFrame, other.m_currentFrame);
        std::swap(m_currentImage, other.m_currentImage);
        std::swap(m_imageNb, other.m_imageNb);
        std::swap(m_images, other.m_images);
        std::swap(m_userImages, other.m_userImages);
        std::swap(m_imageAvailableSemaphores, other.m_imageAvailableSemaphores);
        std::swap(m_renderFinishedSemaphores, other.m_renderFinishedSemaphores);
        std::swap(m_inFlightFences, other.m_inFlightFences);
        std::swap(m_imagesInFlight, other.m_imagesInFlight);

        DeviceObject<VkSwapchainKHR>::operator=(std::move(other));
        return *this;
    }

    Swapchain::~Swapchain()
    {
        cleanup();

        for (std::size_t i = 0; i < m_configuration.maxFramesInFlight; i++)
        {
            vkDestroySemaphore(m_device->getHandle(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_device->getHandle(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_device->getHandle(), m_inFlightFences[i], nullptr);
        }

        for (auto& imageInFlight : m_imagesInFlight)
            imageInFlight = VK_NULL_HANDLE;
    }

    Optional<SwapchainSubmission> Swapchain::getSubmission()
    {
        vkWaitForFences(m_device->getHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        const VkResult result =
            vkAcquireNextImageKHR(m_device->getHandle(), m_handle, UINT64_MAX,
                                  m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentImage);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_device->wait();
            cleanup();
            create();

            return {};
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            logger::error("Failed to acquire swapchain image!");
        }

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_imagesInFlight[m_currentImage] != VK_NULL_HANDLE)
            vkWaitForFences(m_device->getHandle(), 1, &m_imagesInFlight[m_currentImage], VK_TRUE, UINT64_MAX);

        // Mark the image as now being in use by this frame
        m_imagesInFlight[m_currentImage] = m_inFlightFences[m_currentFrame];

        SwapchainSubmission submission;
        submission.imageId        = m_currentImage;
        submission.imageAvailable = m_imageAvailableSemaphores[m_currentFrame];
        submission.renderComplete = m_renderFinishedSemaphores[m_currentFrame];
        submission.frameComplete  = m_inFlightFences[m_currentFrame];
        return submission;
    }

    bool Swapchain::present()
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphores[m_currentFrame];

        const VkSwapchainKHR swapChains[] = {m_handle};
        presentInfo.swapchainCount        = 1;
        presentInfo.pSwapchains           = swapChains;
        presentInfo.pImageIndices         = &m_currentImage;

        const View<Queue> presentQueue = m_device->getPresentQueue();
        const VkResult    result       = vkQueuePresentKHR(presentQueue->getHandle(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            m_device->wait();
            cleanup();
            create();

            return false;
        }

        if (result != VK_SUCCESS)
            logger::error("Failed to present swap chain image!");

        m_currentFrame = (m_currentFrame + 1) % m_configuration.maxFramesInFlight;

        return true;
    }

    void Swapchain::create()
    {
        const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(m_surface->getFormats(m_device));
        m_format                               = static_cast<Format>(surfaceFormat.format);

        const VkPresentModeKHR presentMode = chooseSwapPresentMode(m_surface->getPresentModes(m_device));

        const VkSurfaceCapabilitiesKHR capabilities = m_surface->getCapabilities(m_device);
        const VkExtent2D               vkExtent2D   = chooseExtent(capabilities);
        m_extent                                    = Extent2D{vkExtent2D.width, vkExtent2D.height};

        m_imageNb = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && m_imageNb > capabilities.maxImageCount)
            m_imageNb = capabilities.maxImageCount;
        m_imageNb = std::min(m_configuration.maxFramesInFlight, m_imageNb);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = m_surface->getHandle();
        createInfo.minImageCount    = m_imageNb;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = vkExtent2D;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = toVulkan(ImageUsage::ColorAttachment);

        if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            createInfo.imageUsage |= toVulkan(ImageUsage::TransferDst);
        else
            logger::warn("Device does not allows transfer to swapchain image.");

        if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            createInfo.imageUsage |= toVulkan(ImageUsage::TransferSrc);
        else
            logger::warn("Device does not allows transfer from swapchain image.");

        auto queues = m_device->getQueues();
        if (!m_device->getPresentQueue())
            logger::error("Picked device does not have presentation capabilities.");

        std::vector<uint32_t> queueFamilyIndices{};
        queueFamilyIndices.reserve(queues.size());
        for (const auto& queue : queues)
            queueFamilyIndices.emplace_back(queue->getId());

        SharingMode sharingMode;
        if (queueFamilyIndices.size() == 1)
        {
            sharingMode                      = SharingMode::Exclusive;
            createInfo.imageSharingMode      = toVulkan(sharingMode);
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices   = nullptr;
        }
        else
        {
            sharingMode                      = SharingMode::Concurrent;
            createInfo.imageSharingMode      = toVulkan(sharingMode);
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

        m_images.resize(m_imageNb);
        vkGetSwapchainImagesKHR(m_device->getHandle(), m_handle, &m_imageNb, m_images.data());

        m_userImages.clear();
        m_userImages.reserve(m_imageNb);

        m_format = static_cast<Format>(surfaceFormat.format);
        for (VkImage image : m_images)
            m_userImages.emplace_back(m_device, image, m_extent, ImageUsage::ColorAttachment, m_format, sharingMode);
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
