#include <array>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"

namespace vzt
{
	FrameBuffer::FrameBuffer(vzt::Device* device, const RenderPass* const renderPass,
	                         const std::vector<const vzt::Attachment*>& attachments, vzt::Size2D<uint32_t> size)
	    : m_device(std::move(device)), m_size(std::move(size))
	{
		std::vector<VkImageView> attachmentsViews;
		attachmentsViews.reserve(attachments.size());
		for (const vzt::Attachment* attachment : attachments)
		{
			attachmentsViews.emplace_back(attachment->View()->VkHandle());
		}
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass      = renderPass->VkHandle();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentsViews.size());
		framebufferInfo.pAttachments    = attachmentsViews.data();
		framebufferInfo.width           = m_size.width;
		framebufferInfo.height          = m_size.height;
		framebufferInfo.layers          = 1;

		if (vkCreateFramebuffer(m_device->VkHandle(), &framebufferInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept { std::swap(m_size, other.m_size); }

	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
	{
		std::swap(m_size, other.m_size);
		return *this;
	}

	FrameBuffer::~FrameBuffer()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_device->VkHandle(), m_vkHandle, nullptr);
		}
	}
} // namespace vzt
