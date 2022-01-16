#include <array>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Vazteran/Framework/Vulkan/Device.hpp"
#include "Vazteran/Framework/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Framework/Vulkan/ImageUtils.hpp"
#include "Vazteran/Framework/Vulkan/RenderPass.hpp"

namespace vzt
{
	FrameBuffer::FrameBuffer(vzt::Device* device, const RenderPass* const renderPass, VkImage target,
	                         const vzt::ImageView* const depthImage, VkFormat imageFormat, vzt::Size2D<uint32_t> size)
	    : m_device(device), m_size(std::move(size)), m_target(target)
	{
		m_imageView = m_device->CreateImageView(m_target, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		std::array<VkImageView, 2> attachments = {m_imageView, depthImage->VkHandle()};
		VkFramebufferCreateInfo    framebufferInfo{};
		framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass      = renderPass->VkHandle();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments    = attachments.data();
		framebufferInfo.width           = m_size.width;
		framebufferInfo.height          = m_size.height;
		framebufferInfo.layers          = 1;

		if (vkCreateFramebuffer(m_device->VkHandle(), &framebufferInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
	{
		std::swap(m_size, other.m_size);
		std::swap(m_target, other.m_target);
		std::swap(m_imageView, other.m_imageView);
	}

	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
	{
		std::swap(m_size, other.m_size);
		std::swap(m_target, other.m_target);
		std::swap(m_imageView, other.m_imageView);
		return *this;
	}

	FrameBuffer::~FrameBuffer()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_device->VkHandle(), m_vkHandle, nullptr);
		}

		if (m_imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_device->VkHandle(), m_imageView, nullptr);
		}
	}
} // namespace vzt
