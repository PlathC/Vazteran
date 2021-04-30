#include <stdexcept>
#include <vector>

#include "Vazteran/Vulkan/DeviceManager.hpp"
#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    SwapChain::SwapChain(DeviceManager* deviceManager, VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight) :
            m_frameBufferWidth(frameBufferWidth), m_frameBufferHeight(frameBufferHeight),
            m_deviceManager(deviceManager) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_deviceManager->PhysicalDevice(), surface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR m_presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = FindQueueFamilies(m_deviceManager->PhysicalDevice(), surface);
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

        if (vkCreateSwapchainKHR(m_deviceManager->LogicalDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_deviceManager->LogicalDevice(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_deviceManager->LogicalDevice(), m_swapChain, &imageCount, m_swapChainImages.data());
        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (std::size_t i = 0; i < m_swapChainImageViews.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_deviceManager->LogicalDevice(), &createInfo, nullptr, &m_swapChainImageViews[i])
                != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
        m_graphicPipeline = std::make_unique<GraphicPipeline>(
                m_deviceManager, PipelineSettings{
                        Shader(m_deviceManager, "./shaders/shader.vert.spv", ShaderStage::VertexShader),
                        Shader(m_deviceManager, "./shaders/shader.frag.spv", ShaderStage::FragmentShader),
                        m_swapChainExtent,
                        m_swapChainImageFormat
                }
        );

        for(const auto& imageView : m_swapChainImageViews) {
            m_frameBuffers.emplace_back(std::make_unique<FrameBuffer>(
                    m_deviceManager, m_graphicPipeline->RenderPass(), imageView, m_swapChainExtent.width, m_swapChainExtent.height
            ));
        }

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
        poolInfo.flags = 0; // Optional
        if (vkCreateCommandPool(m_deviceManager->LogicalDevice(), &poolInfo, nullptr, &m_commandPool)
                != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }

        m_commandBuffers.resize(m_frameBuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_deviceManager->LogicalDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }

        for (size_t i = 0; i < m_commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_graphicPipeline->RenderPass();
            renderPassInfo.framebuffer = m_frameBuffers[i]->Buffer();
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_swapChainExtent;

            VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline->Pipeline());
                vkCmdDraw(m_commandBuffers[i], 3, 2, 0, 0);
            vkCmdEndRenderPass(m_commandBuffers[i]);

            if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to record command buffer!");
            }
        }

        m_imageAvailableSemaphores.resize(MaxFramesInFlight);
        m_renderFinishedSemaphores.resize(MaxFramesInFlight);
        m_inFlightFences.resize(MaxFramesInFlight);
        m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MaxFramesInFlight; i++) {
            if (vkCreateSemaphore(m_deviceManager->LogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
                    || vkCreateSemaphore(m_deviceManager->LogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
                    || vkCreateFence(m_deviceManager->LogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS){
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }

    bool SwapChain::DrawFrame() {
        vkWaitForFences(m_deviceManager->LogicalDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_deviceManager->LogicalDevice(), m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return true;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_deviceManager->LogicalDevice(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_deviceManager->LogicalDevice(), 1, &m_inFlightFences[m_currentFrame]);

        if (vkQueueSubmit(m_deviceManager->GraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(m_deviceManager->PresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            return true;
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;
        return false;
    }

    SwapChain::~SwapChain() {
        for (std::size_t i = 0; i < MaxFramesInFlight; i++) {
            vkDestroySemaphore(m_deviceManager->LogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_deviceManager->LogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_deviceManager->LogicalDevice(), m_inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(m_deviceManager->LogicalDevice(), m_commandPool, nullptr);

        for (auto imageView : m_swapChainImageViews) {
            vkDestroyImageView(m_deviceManager->LogicalDevice(), imageView, nullptr);
        }

        vkDestroySwapchainKHR(m_deviceManager->LogicalDevice(), m_swapChain, nullptr);
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
            return {
                    static_cast<uint32_t>(m_frameBufferWidth),
                    static_cast<uint32_t>(m_frameBufferHeight)
            };
        }
    }
}