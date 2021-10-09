#include <stdexcept>
#include <utility>
#include <vector>

#include "Vazteran/Vulkan/CommandPool.hpp"
#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/RenderPass.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    SwapChain::SwapChain(vzt::LogicalDevice* logicalDevice, VkSurfaceKHR surface, vzt::Size2D<int> frameBufferSize,
                         CommandPool* commandPool, uint32_t maxFrameInFlight) :
            m_frameBufferSize(std::move(frameBufferSize)), m_surface(surface), m_logicalDevice(logicalDevice),
            m_commandPool(commandPool), m_maxFrameInFlight(maxFrameInFlight) {
        CreateSwapChain();
        CreateDepthResources();
        CreateRenderSupport();
        CreateSynchronizationObjects();
    }

    SwapChain::SwapChain(SwapChain&& other) noexcept {
        m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));m_currentFrame = std::exchange(other.m_currentFrame, 0);
        m_framebufferResized = std::exchange(other.m_framebufferResized, false);
        m_frameBufferSize = std::exchange(other.m_frameBufferSize, {});
        std::exchange(m_swapChainImageFormat, other.m_swapChainImageFormat);
        m_commandPool = std::exchange(other.m_commandPool, nullptr);
        m_depthImage = std::exchange(other.m_depthImage, nullptr);

        std::swap(m_surface, other.m_surface);
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_graphicPipelines, other.m_graphicPipelines);
        std::swap(m_swapChainExtent, other.m_swapChainExtent);
        std::swap(m_imageAvailableSemaphores, other.m_imageAvailableSemaphores);
        std::swap(m_renderFinishedSemaphores, other.m_renderFinishedSemaphores);
        std::swap(m_inFlightFences, other.m_inFlightFences);
        std::swap(m_imagesInFlight, other.m_imagesInFlight);
        std::swap(m_frames, other.m_frames);
        std::swap(m_commandPool, other.m_commandPool);
        std::swap(m_maxFrameInFlight, other.m_maxFrameInFlight);
    }

    SwapChain& SwapChain::operator=(SwapChain&& other) noexcept {
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_vkHandle, other.m_vkHandle);
        std::swap(m_currentFrame, other.m_currentFrame);
        std::swap(m_framebufferResized, other.m_framebufferResized);
        std::swap(m_frameBufferSize, other.m_frameBufferSize);
        std::swap(m_imageCount, other.m_imageCount);
        std::swap(m_swapChainImageFormat, other.m_swapChainImageFormat);
        std::swap(m_swapChainExtent, other.m_swapChainExtent);
        std::swap(m_commandPool, other.m_commandPool);
        std::swap(m_imageAvailableSemaphores, other.m_imageAvailableSemaphores);
        std::swap(m_renderFinishedSemaphores, other.m_renderFinishedSemaphores);
        std::swap(m_inFlightFences, other.m_inFlightFences);
        std::swap(m_imagesInFlight, other.m_imagesInFlight);
        std::swap(m_depthImage, other.m_depthImage);
        std::swap(m_frames, other.m_frames);
        std::swap(m_surface, other.m_surface);
        std::swap(m_commandPool, other.m_commandPool);
        std::swap(m_maxFrameInFlight, other.m_maxFrameInFlight);

        return *this;
    }

    bool SwapChain::DrawFrame() {
        vkWaitForFences(m_logicalDevice->VkHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_logicalDevice->VkHandle(), m_vkHandle, UINT64_MAX,
                                                m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return true;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_logicalDevice->VkHandle(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

        m_commandPool->RecordBuffer(m_swapChainExtent, m_graphicPipelines[0].get(), m_frames[imageIndex], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(*m_commandPool)[imageIndex];

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_logicalDevice->VkHandle(), 1, &m_inFlightFences[m_currentFrame]);

        if (vkQueueSubmit(m_logicalDevice->GraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_vkHandle };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(m_logicalDevice->PresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
            m_framebufferResized = false;
            return true;
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        m_currentFrame = (m_currentFrame + 1) % m_maxFrameInFlight;
        return false;
    }


    void SwapChain::Recreate(VkSurfaceKHR surface) {
        Cleanup();

        m_surface = surface;

        CreateSwapChain();
        CreateDepthResources();
        CreateRenderSupport();
    }

    void SwapChain::FrameBufferResized(vzt::Size2D<int> newSize) {
        m_frameBufferSize = newSize;
        m_framebufferResized = true;
    };

    vzt::Size2D<int> SwapChain::FrameBufferSize() const {
        return m_frameBufferSize;
    }

    SwapChain::~SwapChain() {
        for (std::size_t i = 0; i < m_maxFrameInFlight; i++) {
            vkDestroySemaphore(m_logicalDevice->VkHandle(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_logicalDevice->VkHandle(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_logicalDevice->VkHandle(), m_inFlightFences[i], nullptr);
        }

        for (auto& imageInFlight : m_imagesInFlight)
            imageInFlight = VK_NULL_HANDLE;

        Cleanup();
    }

    void SwapChain::CreateSwapChain() {
        SwapChainSupportDetails swapChainSupport = m_logicalDevice->Parent()->QuerySwapChainSupport(m_surface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR m_presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        m_imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && m_imageCount > swapChainSupport.capabilities.maxImageCount) {
            m_imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = m_imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        QueueFamilyIndices indices = m_logicalDevice->DeviceQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = m_presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_logicalDevice->VkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain!");
        }

        m_graphicPipelines.emplace_back(std::make_unique<vzt::GraphicPipeline>(
                m_logicalDevice, PipelineSettings{
                        std::make_unique<vzt::RenderPass>(m_logicalDevice, m_swapChainImageFormat),
                        m_swapChainExtent
                }
        ));
    }

    void SwapChain::CreateDepthResources() {
        VkFormat depthFormat = m_logicalDevice->Parent()->FindDepthFormat();
        m_depthImage = std::make_unique<vzt::ImageHandler>( vzt::ImageHandler{
                vzt::ImageView(m_logicalDevice, Size2D<uint32_t>{ m_swapChainExtent.width, m_swapChainExtent.height },
                               depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
                               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
                Sampler(m_logicalDevice) });
    }

    void SwapChain::CreateRenderSupport() {
        vkGetSwapchainImagesKHR(m_logicalDevice->VkHandle(), m_vkHandle, &m_imageCount, nullptr);
        auto swapChainImages = std::vector<VkImage>(m_imageCount);
        m_frames.reserve(m_imageCount);
        vkGetSwapchainImagesKHR(m_logicalDevice->VkHandle(), m_vkHandle, &m_imageCount, swapChainImages.data());
        m_commandPool->AllocateCommandBuffers(m_imageCount);

        auto tempImageData = std::vector<uint8_t>(m_swapChainExtent.width * m_swapChainExtent.height * 4, 0);
        for (std::size_t i = 0; i < m_imageCount; i++) {
            // TODO: Handle CommandBuffer in a CommandPool class
            VkImageView handle = m_logicalDevice->CreateImageView(swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

            auto frameBuffer = vzt::FrameBuffer(m_logicalDevice, swapChainImages[i], handle, m_graphicPipelines[0]->RenderPass()->VkHandle(),
                                                std::vector<VkImageView>({ handle, m_depthImage->imageView.VkHandle() }),
                                                m_swapChainExtent.width, m_swapChainExtent.height);

            m_frames.emplace_back(std::move(frameBuffer));
            m_commandPool->RecordBuffer(m_swapChainExtent, m_graphicPipelines[0].get(), m_frames[i], static_cast<uint32_t>(i));
        }
    }

    void SwapChain::CreateSynchronizationObjects() {
        m_imageAvailableSemaphores.resize(m_maxFrameInFlight);
        m_renderFinishedSemaphores.resize(m_maxFrameInFlight);
        m_inFlightFences.resize(m_maxFrameInFlight);
        m_imagesInFlight.resize(m_imageCount, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (std::size_t i = 0; i < m_maxFrameInFlight; i++) {
            if (vkCreateSemaphore(m_logicalDevice->VkHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
                || vkCreateSemaphore(m_logicalDevice->VkHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
                || vkCreateFence(m_logicalDevice->VkHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS){
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }

    void SwapChain::Cleanup() {
        for (std::size_t i = 0; i < m_frames.size(); i++) {
            if (m_imagesInFlight[i] != VK_NULL_HANDLE) {
                vkWaitForFences(m_logicalDevice->VkHandle(), 1, &m_imagesInFlight[i], VK_TRUE, UINT64_MAX);
            }
        }
        m_frames.clear();
        m_depthImage.reset();

        vkDestroySwapchainKHR(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);

        m_graphicPipelines.clear();
    }

    VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            return { static_cast<uint32_t>(m_frameBufferSize.width),
                     static_cast<uint32_t>(m_frameBufferSize.height) };
        }
    }
}