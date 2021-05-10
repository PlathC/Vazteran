#include <stdexcept>
#include <utility>
#include <vector>

#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/RenderPass.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    SwapChain::SwapChain(vzt::LogicalDevice* logicalDevice, VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight,
                         vzt::RenderPassFunction renderPass, std::unordered_set<vzt::Shader, vzt::ShaderHash> shaders) :
            m_frameBufferWidth(frameBufferWidth), m_frameBufferHeight(frameBufferHeight), m_renderPass(std::move(renderPass)),
            m_surface(surface), m_logicalDevice(logicalDevice), m_shaders(std::move(shaders)) {
        CreateSwapChain();
        CreateDepthResources();
        CreateImageKHR();
        CreateSynchronizationObjects();
    }

    SwapChain::SwapChain(SwapChain&& other) noexcept {
        m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));m_currentFrame = std::exchange(other.m_currentFrame, 0);
        m_framebufferResized = std::exchange(other.m_framebufferResized, false);
        m_frameBufferWidth = std::exchange(other.m_frameBufferWidth, 0);
        m_frameBufferHeight = std::exchange(other.m_frameBufferHeight, 0);m_imageCount = std::exchange(other.m_imageCount, 0);
        m_swapChainImageFormat = std::exchange(other.m_swapChainImageFormat, static_cast<decltype(m_swapChainImageFormat)>(VK_NULL_HANDLE));
        m_commandPool = std::exchange(other.m_commandPool, static_cast<decltype(m_commandPool)>(VK_NULL_HANDLE));
        m_descriptorPool = std::exchange(other.m_descriptorPool, static_cast<decltype(m_descriptorPool)>(VK_NULL_HANDLE));
        m_depthImage = std::exchange(other.m_depthImage, nullptr);

        std::swap(m_surface, other.m_surface);
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_graphicPipeline, other.m_graphicPipeline);
        std::swap(m_renderPass, other.m_renderPass);
        std::swap(m_swapChainExtent, other.m_swapChainExtent);
        std::swap(m_imageAvailableSemaphores, other.m_imageAvailableSemaphores);
        std::swap(m_renderFinishedSemaphores, other.m_renderFinishedSemaphores);
        std::swap(m_inFlightFences, other.m_inFlightFences);
        std::swap(m_imagesInFlight, other.m_imagesInFlight);
        std::swap(m_frames, other.m_frames);
        std::swap(m_shaders, other.m_shaders);
    }

    SwapChain& SwapChain::operator=(SwapChain&& other) noexcept {
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_vkHandle, other.m_vkHandle);
        std::swap(m_graphicPipeline, other.m_graphicPipeline);
        std::swap(m_currentFrame, other.m_currentFrame);
        std::swap(m_framebufferResized, other.m_framebufferResized);
        std::swap(m_frameBufferWidth, other.m_frameBufferWidth);
        std::swap(m_frameBufferHeight, other.m_frameBufferHeight);
        std::swap(m_renderPass, other.m_renderPass);
        std::swap(m_imageCount, other.m_imageCount);
        std::swap(m_swapChainImageFormat, other.m_swapChainImageFormat);
        std::swap(m_swapChainExtent, other.m_swapChainExtent);
        std::swap(m_commandPool, other.m_commandPool);
        std::swap(m_descriptorPool, other.m_descriptorPool);
        std::swap(m_imageAvailableSemaphores, other.m_imageAvailableSemaphores);
        std::swap(m_renderFinishedSemaphores, other.m_renderFinishedSemaphores);
        std::swap(m_inFlightFences, other.m_inFlightFences);
        std::swap(m_imagesInFlight, other.m_imagesInFlight);
        std::swap(m_depthImage, other.m_depthImage);
        std::swap(m_frames, other.m_frames);
        std::swap(m_shaders, other.m_shaders);
        std::swap(m_surface, other.m_surface);

        return *this;
    }

    bool SwapChain::DrawFrame(vzt::Transforms ubo) {
        vkWaitForFences(m_logicalDevice->VkHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_logicalDevice->VkHandle(), m_vkHandle, UINT64_MAX,
                                                m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return true;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        UpdateUniformBuffer(imageIndex, ubo);

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_logicalDevice->VkHandle(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
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
        submitInfo.pCommandBuffers = &m_frames[imageIndex].commandBuffer;

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

        m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;

        return false;
    }


    void SwapChain::Recreate(VkSurfaceKHR surface) {
        Cleanup();

        m_surface = surface;

        CreateSwapChain();
        CreateDepthResources();
        CreateImageKHR();
    }

    void SwapChain::FrameBufferResized(vzt::Size2D newSize) {
        m_frameBufferWidth = newSize.width;
        m_frameBufferHeight = newSize.height;
        m_framebufferResized = true;
    };

    vzt::Size2D SwapChain::FrameBufferSize() const {
        return { static_cast<uint32_t>(m_frameBufferWidth),
                 static_cast<uint32_t>(m_frameBufferHeight) };
    }

    SwapChain::~SwapChain() {
        for (std::size_t i = 0; i < MaxFramesInFlight; i++) {
            vkDestroySemaphore(m_logicalDevice->VkHandle(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_logicalDevice->VkHandle(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_logicalDevice->VkHandle(), m_inFlightFences[i], nullptr);
        }

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
            throw std::runtime_error("Failed to create swap chain!");
        }
        m_graphicPipeline = std::make_unique<vzt::GraphicPipeline>(
                m_logicalDevice, PipelineSettings{
                        m_shaders,
                        std::make_unique<vzt::RenderPass>(m_logicalDevice, m_swapChainImageFormat),
                        m_swapChainExtent
                }
        );
    }

    void SwapChain::CreateImageKHR() {
        vkGetSwapchainImagesKHR(m_logicalDevice->VkHandle(), m_vkHandle, &m_imageCount, nullptr);
        auto swapChainImages = std::vector<VkImage>(m_imageCount);
        m_frames.reserve(m_imageCount);
        vkGetSwapchainImagesKHR(m_logicalDevice->VkHandle(), m_vkHandle, &m_imageCount, swapChainImages.data());

        auto descriptorTypes = m_graphicPipeline->DescriptorTypes();
        auto poolSizes = std::vector<VkDescriptorPoolSize>(descriptorTypes.size());

        for (std::size_t i = 0; i < descriptorTypes.size(); i++) {
            poolSizes[i].type = descriptorTypes[i];
            poolSizes[i].descriptorCount = static_cast<uint32_t>(m_imageCount);
        }

        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = static_cast<uint32_t>(m_imageCount);

        if (vkCreateDescriptorPool(m_logicalDevice->VkHandle(), &descriptorPoolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
        auto layouts = std::vector<VkDescriptorSetLayout>(m_imageCount, m_graphicPipeline->DescriptorSetLayout());
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = m_descriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(m_imageCount);
        descriptorSetAllocInfo.pSetLayouts = layouts.data();

        auto descriptorSets = std::vector<VkDescriptorSet>(m_imageCount);
        if (vkAllocateDescriptorSets(m_logicalDevice->VkHandle(), &descriptorSetAllocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        vzt::QueueFamilyIndices indices = m_logicalDevice->DeviceQueueFamilyIndices();

        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();
        commandPoolInfo.flags = 0; // Optional
        if (vkCreateCommandPool(m_logicalDevice->VkHandle(), &commandPoolInfo, nullptr, &m_commandPool)
            != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }

        auto commandsBuffers = std::vector<VkCommandBuffer>(m_imageCount);
        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool = m_commandPool;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(commandsBuffers.size());

        if (vkAllocateCommandBuffers(m_logicalDevice->VkHandle(), &commandBufferAllocInfo, commandsBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }

        auto tempImageData = std::vector<uint8_t>(m_swapChainExtent.width * m_swapChainExtent.height * 4, 0);
        for (size_t i = 0; i < m_imageCount; i++) {
            // TODO: Handle CommandBuffer in a CommandPool class
            auto buffer = vzt::Buffer<vzt::Transforms>(m_logicalDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            VkImageView handle = m_logicalDevice->CreateImageView(swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

            auto frameBuffer = vzt::FrameBuffer(m_logicalDevice, m_graphicPipeline->RenderPass()->VkHandle(),
                                                std::vector<VkImageView>({ handle, m_depthImage->imageView.VkHandle() }),
                                                m_swapChainExtent.width, m_swapChainExtent.height);

            m_graphicPipeline->UpdateDescriptorSet(descriptorSets[i], buffer.VkHandle());

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(commandsBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_graphicPipeline->RenderPass()->VkHandle();
            renderPassInfo.framebuffer = frameBuffer.VkHandle();
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandsBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(commandsBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline->VkHandle());
                m_renderPass(commandsBuffers[i], descriptorSets[i], m_graphicPipeline.get());
            vkCmdEndRenderPass(commandsBuffers[i]);

            if (vkEndCommandBuffer(commandsBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to record command buffer!");
            }

            m_frames.emplace_back( FrameComponent{
                    descriptorSets[i], commandsBuffers[i], swapChainImages[i], handle,
                    std::move(frameBuffer), std::move(buffer)
            });
        }
    }


    void SwapChain::CreateDepthResources() {
        VkFormat depthFormat = m_logicalDevice->Parent()->FindDepthFormat();
        m_depthImage = std::make_unique<vzt::ImageHandler>( vzt::ImageHandler{
                vzt::ImageView(m_logicalDevice, Size2D{ m_swapChainExtent.width, m_swapChainExtent.height },
                               depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
                               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
                Sampler(m_logicalDevice) });
    }

    void SwapChain::CreateSynchronizationObjects() {
        m_imageAvailableSemaphores.resize(MaxFramesInFlight);
        m_renderFinishedSemaphores.resize(MaxFramesInFlight);
        m_inFlightFences.resize(MaxFramesInFlight);
        m_imagesInFlight.resize(m_imageCount, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MaxFramesInFlight; i++) {
            if (vkCreateSemaphore(m_logicalDevice->VkHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
                || vkCreateSemaphore(m_logicalDevice->VkHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
                || vkCreateFence(m_logicalDevice->VkHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS){
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }

    void SwapChain::UpdateUniformBuffer(uint32_t currentImage, vzt::Transforms ubo) {
        m_frames[currentImage].uniformBuffer.Update<vzt::Transforms>({ubo});
    }

    void SwapChain::Cleanup() {
        vkDeviceWaitIdle(m_logicalDevice->VkHandle());

        m_depthImage.reset();
        auto commandsBuffers = std::vector<VkCommandBuffer>(m_imageCount);
        for (std::size_t i = 0; i < m_frames.size(); i++) {
            commandsBuffers[i] = m_frames[i].commandBuffer;
            vkDestroyImageView(m_logicalDevice->VkHandle(), m_frames[i].colorImageView, nullptr);
        }

        vkFreeCommandBuffers(m_logicalDevice->VkHandle(), m_commandPool, static_cast<uint32_t>(commandsBuffers.size()), commandsBuffers.data());
        vkDestroyDescriptorPool(m_logicalDevice->VkHandle(), m_descriptorPool, nullptr);
        vkDestroyCommandPool(m_logicalDevice->VkHandle(), m_commandPool, nullptr);
        vkDestroySwapchainKHR(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);

        m_frames.clear();
        m_graphicPipeline.reset();

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