#include <array>
#include <stdexcept>

#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	RenderPass::RenderPass(vzt::Device* device, std::size_t subpassCount, std::vector<VkSubpassDependency>&& subpasses,
	                       const std::vector<vzt::Attachment*>& attachments)
	    : m_device(device), m_subpassDependencies(std::move(subpasses))
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		attachmentDescriptions.reserve(attachments.size());

		m_colorRefs.reserve(attachments.size() - 1);
		for (std::size_t i = 0; i < attachments.size(); i++)
		{
			const auto& attachment = attachments[i];
			attachmentDescriptions.emplace_back(attachment->getDescription());

			const auto attachmentLayout = attachment->getLayout();

			VkAttachmentReference currentAttachmentRef{};
			currentAttachmentRef.attachment = static_cast<uint32_t>(i);
			currentAttachmentRef.layout     = static_cast<VkImageLayout>(attachmentLayout);

			if (attachmentLayout == vzt::ImageLayout::ColorAttachmentOptimal)
			{
				m_colorRefs.emplace_back(currentAttachmentRef);
			}
			else if (attachmentLayout == vzt::ImageLayout::DepthAttachmentOptimal ||
			         attachmentLayout == vzt::ImageLayout::DepthStencilAttachmentOptimal)
			{
				m_depthRef = currentAttachmentRef;
			}
		}

		m_subpassDescriptions.reserve(subpassCount);
		for (std::size_t i = 0; i < subpassCount; i++)
		{
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount    = static_cast<uint32_t>(m_colorRefs.size());
			subpass.pColorAttachments       = m_colorRefs.data();
			subpass.pDepthStencilAttachment = &m_depthRef;

			m_subpassDescriptions.emplace_back(subpass);
		}

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments    = attachmentDescriptions.data();
		renderPassInfo.subpassCount    = static_cast<uint32_t>(m_subpassDescriptions.size());
		renderPassInfo.pSubpasses      = m_subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(m_subpassDependencies.size());
		renderPassInfo.pDependencies   = m_subpassDependencies.data();

		if (vkCreateRenderPass(m_device->VkHandle(), &renderPassInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass!");
		}
	}

	RenderPass::RenderPass(RenderPass&& other) noexcept
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_device, other.m_device);
	}

	RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_device, other.m_device);

		return *this;
	}

	RenderPass::~RenderPass()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_device->VkHandle(), m_vkHandle, nullptr);
		}
	}

	void RenderPass::bind(VkCommandBuffer commandBuffer, const vzt::FrameBuffer* const frameBuffer) const
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass        = m_vkHandle;
		renderPassInfo.framebuffer       = frameBuffer->vkHandle();
		renderPassInfo.renderArea.offset = {0, 0};

		const auto fbSize                = frameBuffer->size();
		renderPassInfo.renderArea.extent = VkExtent2D{fbSize.width, fbSize.height};

		std::vector<VkClearValue> clearValues;
		clearValues.resize(m_colorRefs.size() + 1);
		for (std::size_t i = 0; i < m_colorRefs.size(); i++)
		{
			clearValues[m_colorRefs[i].attachment].color = {0.0f, 0.0f, 0.0f, 1.0f};
		}
		clearValues[m_depthRef.attachment].depthStencil = {1.0f, 0};

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues    = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::unbind(VkCommandBuffer commandBuffer) const { vkCmdEndRenderPass(commandBuffer); }
} // namespace vzt
