#include <vulkan/vulkan.h>

#include "Vazteran/Vulkan/DeviceManager.hpp"
#include "Vazteran/Vulkan/FrameBuffer.hpp"

namespace vzt {
    FrameBuffer::FrameBuffer(DeviceManager* deviceManager, VkRenderPass renderPass, VkImageView imageView,
                             uint32_t width, uint32_t height):
            m_deviceManager(deviceManager) {
        VkImageView attachments[] = {
                imageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(deviceManager->LogicalDevice(), &framebufferInfo, nullptr, &m_frameBuffer)
            != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }

    FrameBuffer::~FrameBuffer() {
        vkDestroyFramebuffer(m_deviceManager->LogicalDevice(), m_frameBuffer, nullptr);
    }
}