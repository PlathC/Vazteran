#include <array>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/FrameBuffer.hpp"

namespace vzt {
    FrameBuffer::FrameBuffer(LogicalDevice* logicalDevice, VkRenderPass renderPass, std::vector<VkImageView> attachments, uint32_t width, uint32_t height):
            m_logicalDevice(logicalDevice) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_logicalDevice->VkHandle(), &framebufferInfo, nullptr, &m_vkHandle)
            != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }

    FrameBuffer::~FrameBuffer() {
        vkDestroyFramebuffer(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
    }
}