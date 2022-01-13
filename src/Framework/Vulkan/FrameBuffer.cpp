#include <array>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Vazteran/Framework/Vulkan/Device.hpp"
#include "Vazteran/Framework/Vulkan/FrameBuffer.hpp"

namespace vzt
{
	FrameBuffer::FrameBuffer(
	    vzt::Device *logicalDevice, VkImage image, VkImageView imageView, VkRenderPass renderPass,
	    std::vector<VkImageView> attachments, uint32_t width, uint32_t height)
	    : m_logicalDevice(logicalDevice), m_image(image), m_imageView(imageView)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_logicalDevice->VkHandle(), &framebufferInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	FrameBuffer::FrameBuffer(FrameBuffer &&other) noexcept
	{
		m_logicalDevice = std::exchange(other.m_logicalDevice, nullptr);
		m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		std::swap(m_image, other.m_image);
		m_imageView = std::exchange(other.m_imageView, static_cast<decltype(m_imageView)>(VK_NULL_HANDLE));
	}

	FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other) noexcept
	{
		std::swap(m_logicalDevice, other.m_logicalDevice);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_image, other.m_image);
		std::swap(m_imageView, other.m_imageView);

		return *this;
	}

	FrameBuffer::~FrameBuffer()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
		}

		if (m_imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_logicalDevice->VkHandle(), m_imageView, nullptr);
		}
	}
} // namespace vzt
