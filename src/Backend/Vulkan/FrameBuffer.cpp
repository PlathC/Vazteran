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
	FrameBuffer::FrameBuffer(vzt::Device* device, std::size_t subpassCount,
	                         std::vector<VkSubpassDependency>&& subpassDependencies,
	                         std::vector<vzt::Attachment>&& attachments, vzt::Size2D<uint32_t> size)
	    : m_device(device), m_size(size), m_attachments(std::move(attachments))
	{
		std::vector<vzt::Attachment*> attachmentCopy;
		attachmentCopy.reserve(m_attachments.size());
		std::vector<VkImageView> attachmentsViews;
		attachmentsViews.reserve(m_attachments.size());
		for (auto& attachment : m_attachments)
		{
			attachmentsViews.emplace_back(attachment.getView()->vkHandle());
			attachmentCopy.emplace_back(&attachment);
		}

		m_renderPass =
		    std::make_unique<vzt::RenderPass>(m_device, subpassCount, std::move(subpassDependencies), attachmentCopy);

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass      = m_renderPass->vkHandle();
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

	FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_size, other.m_size);
		std::swap(m_attachments, other.m_attachments);
		std::swap(m_renderPass, other.m_renderPass);
	}

	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_size, other.m_size);
		std::swap(m_attachments, other.m_attachments);
		std::swap(m_renderPass, other.m_renderPass);

		return *this;
	}

	FrameBuffer::~FrameBuffer()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_device->VkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

	void FrameBuffer::bind(VkCommandBuffer commandBuffer) const
	{
		m_renderPass->bind(commandBuffer, this);
		VkViewport viewport;
		viewport.x        = 0.f;
		viewport.y        = 0.f;
		viewport.width    = static_cast<float>(m_size.width);
		viewport.height   = static_cast<float>(m_size.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.extent = {m_size.width, m_size.height};
		scissor.offset = {0, 0};
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void FrameBuffer::unbind(VkCommandBuffer commandBuffer) const { m_renderPass->unbind(commandBuffer); }
} // namespace vzt
