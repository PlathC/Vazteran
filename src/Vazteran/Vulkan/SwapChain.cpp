#include <stdexcept>
#include <utility>
#include <vector>

#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/PhysicalDevice.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    SwapChain::SwapChain(LogicalDevice* logicalDevice, VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight,
                         RenderPassFunction renderPass, const PhongMaterial& currentMaterial) :
            m_frameBufferWidth(frameBufferWidth), m_frameBufferHeight(frameBufferHeight), m_renderPass(std::move(renderPass)),
            m_surface(surface), m_logicalDevice(logicalDevice) {

        m_ambientImage =std::make_unique<TextureImage>(logicalDevice, currentMaterial.ambient);
        m_ambientSampler = std::make_unique<TextureSampler>(m_logicalDevice);
        m_diffuseImage = std::make_unique<TextureImage>(logicalDevice, currentMaterial.diffuse);
        m_diffuseSampler = std::make_unique<TextureSampler>(m_logicalDevice);
        m_specularImage = std::make_unique<TextureImage>(logicalDevice, currentMaterial.specular);
        m_specularSampler = std::make_unique<TextureSampler>(m_logicalDevice);
        CreateSwapChain();
        CreateImageKHR();
        CreateImageViews();
        CreateDepthResources();
        CreateFrameBuffers();
        CreateCommandBuffers();
        CreateSynchronizationObjects();
    }

    bool SwapChain::DrawFrame(UniformBufferObject ubo) {
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
        submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

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

        VkSwapchainKHR swapChains[] = {m_vkHandle };
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


    void SwapChain::Recreate(VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight) {
        Cleanup();

        m_surface = surface;
        m_frameBufferWidth = frameBufferWidth;
        m_frameBufferHeight = frameBufferHeight;

        CreateSwapChain();
        CreateImageKHR();
        CreateImageViews();
        CreateDepthResources();
        CreateFrameBuffers();
        CreateCommandBuffers();
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

        m_graphicPipeline = std::make_unique<GraphicPipeline>(
                m_logicalDevice, PipelineSettings{
                        Shader(m_logicalDevice, "./shaders/shader.vert.spv", ShaderStage::VertexShader),
                        Shader(m_logicalDevice, "./shaders/shader.frag.spv", ShaderStage::FragmentShader),
                        m_swapChainExtent,
                        m_swapChainImageFormat
                }
        );
    }

    void SwapChain::CreateImageKHR() {
        vkGetSwapchainImagesKHR(m_logicalDevice->VkHandle(), m_vkHandle, &m_imageCount, nullptr);
        m_swapChainImages.resize(m_imageCount);
        vkGetSwapchainImagesKHR(m_logicalDevice->VkHandle(), m_vkHandle, &m_imageCount, m_swapChainImages.data());

        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(m_swapChainImages.size());
        m_uniformBuffersMemory.resize(m_swapChainImages.size());
        for (size_t i = 0; i < m_swapChainImages.size(); i++) {
            m_logicalDevice->CreateBuffer(
                    m_uniformBuffers[i], m_uniformBuffersMemory[i],
                    bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
        }

        std::array<VkDescriptorPoolSize, 4> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
        poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[3].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(m_swapChainImages.size());

        if (vkCreateDescriptorPool(m_logicalDevice->VkHandle(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        auto layouts = std::vector<VkDescriptorSetLayout>(m_swapChainImages.size(), m_graphicPipeline->DescriptorSetLayout());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainImages.size());
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(m_swapChainImages.size());
        if (vkAllocateDescriptorSets(m_logicalDevice->VkHandle(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        UpdateDescriptorSets();
    }

    void SwapChain::CreateImageViews() {
        m_swapChainImageViews.resize(m_swapChainImages.size());
        for (std::size_t i = 0; i < m_swapChainImageViews.size(); i++) {
            m_swapChainImageViews[i] = m_logicalDevice->CreateImageView(m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void SwapChain::CreateDepthResources() {
        VkFormat depthFormat = m_logicalDevice->Parent()->FindDepthFormat();
        m_logicalDevice->CreateImage(
                m_depthImage, m_depthImageMemory,
                m_swapChainExtent.width, m_swapChainExtent.height, depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_depthImageView = m_logicalDevice->CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        m_logicalDevice->TransitionImageLayout(
                m_depthImage, depthFormat,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void SwapChain::CreateFrameBuffers() {
        for(const auto& imageView : m_swapChainImageViews) {
            m_frameBuffers.emplace_back(std::make_unique<FrameBuffer>(
                    m_logicalDevice, m_graphicPipeline->RenderPass(), std::vector<VkImageView>({imageView, m_depthImageView}),
                    m_swapChainExtent.width, m_swapChainExtent.height
            ));
        }
    }

    void SwapChain::CreateCommandBuffers() {
        QueueFamilyIndices indices = m_logicalDevice->DeviceQueueFamilyIndices();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
        poolInfo.flags = 0; // Optional
        if (vkCreateCommandPool(m_logicalDevice->VkHandle(), &poolInfo, nullptr, &m_commandPool)
            != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }

        m_commandBuffers.resize(m_frameBuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_logicalDevice->VkHandle(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
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
            renderPassInfo.framebuffer = m_frameBuffers[i]->VkHandle();
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline->VkHandle());
                m_renderPass(m_commandBuffers[i], m_descriptorSets[i], m_graphicPipeline.get());
            vkCmdEndRenderPass(m_commandBuffers[i]);

            if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to record command buffer!");
            }
        }
    }

    void SwapChain::CreateSynchronizationObjects() {
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
            if (vkCreateSemaphore(m_logicalDevice->VkHandle(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
                || vkCreateSemaphore(m_logicalDevice->VkHandle(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
                || vkCreateFence(m_logicalDevice->VkHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS){
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }

    void SwapChain::UpdateUniformBuffer(uint32_t currentImage, UniformBufferObject ubo) {
        void* data;
        vkMapMemory(m_logicalDevice->VkHandle(), m_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(m_logicalDevice->VkHandle(), m_uniformBuffersMemory[currentImage]);
    }

    void SwapChain::UpdateDescriptorSets() {
        for (std::size_t i = 0; i < m_swapChainImages.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo ambientInfo{};
            ambientInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            ambientInfo.imageView = m_ambientImage->ImageView();
            ambientInfo.sampler = m_ambientSampler->VkHandle();

            VkDescriptorImageInfo diffuseInfo{};
            diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            diffuseInfo.imageView = m_diffuseImage->ImageView();
            diffuseInfo.sampler = m_diffuseSampler->VkHandle();

            VkDescriptorImageInfo specularInfo{};
            specularInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            specularInfo.imageView = m_specularImage->ImageView();
            specularInfo.sampler = m_specularSampler->VkHandle();

            std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &ambientInfo;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = m_descriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pImageInfo = &diffuseInfo;

            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[3].dstSet = m_descriptorSets[i];
            descriptorWrites[3].dstBinding = 3;
            descriptorWrites[3].dstArrayElement = 0;
            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[3].descriptorCount = 1;
            descriptorWrites[3].pImageInfo = &specularInfo;

            vkUpdateDescriptorSets(m_logicalDevice->VkHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void SwapChain::Cleanup() {
        vkDeviceWaitIdle(m_logicalDevice->VkHandle());

        vkDestroyImageView(m_logicalDevice->VkHandle(), m_depthImageView, nullptr);
        vkDestroyImage(m_logicalDevice->VkHandle(), m_depthImage, nullptr);
        vkFreeMemory(m_logicalDevice->VkHandle(), m_depthImageMemory, nullptr);

        m_frameBuffers.clear();
        vkFreeCommandBuffers(m_logicalDevice->VkHandle(), m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_graphicPipeline.reset();

        vkDestroyCommandPool(m_logicalDevice->VkHandle(), m_commandPool, nullptr);

        for (auto& m_swapChainImageView : m_swapChainImageViews) {
            vkDestroyImageView(m_logicalDevice->VkHandle(), m_swapChainImageView, nullptr);
        }

        vkDestroySwapchainKHR(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);

        for (size_t i = 0; i < m_swapChainImages.size(); i++) {
            vkDestroyBuffer(m_logicalDevice->VkHandle(), m_uniformBuffers[i], nullptr);
            vkFreeMemory(m_logicalDevice->VkHandle(), m_uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(m_logicalDevice->VkHandle(), m_descriptorPool, nullptr);
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