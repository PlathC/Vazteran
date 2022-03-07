#include <array>
#include <stdexcept>

#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	RenderPass::RenderPass(const vzt::Device* device, const std::vector<AttachmentPassConfiguration>& attachments,
	                       const DepthAttachmentPassConfiguration& depthAttachment)
	    : m_device(device)
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		attachmentDescriptions.reserve(attachments.size());

		m_colorRefs.reserve(attachments.size() - 1);
		for (std::size_t i = 0; i < attachments.size(); i++)
		{
			const auto& attachment = attachments[i];

			VkAttachmentDescription description{};
			description.initialLayout  = vzt::toVulkan(attachment.second.initialLayout);
			description.finalLayout    = vzt::toVulkan(attachment.second.finalLayout);
			description.stencilLoadOp  = vzt::toVulkan(attachment.second.stencilLoapOp);
			description.stencilStoreOp = vzt::toVulkan(attachment.second.stencilStoreOp);
			description.loadOp         = vzt::toVulkan(attachment.second.loadOp);
			description.storeOp        = vzt::toVulkan(attachment.second.storeOp);
			description.format         = vzt::toVulkan(attachment.first->getFormat());
			description.samples        = vzt::toVulkan(attachment.first->getSampleCount());
			attachmentDescriptions.emplace_back(description);

			const auto attachmentLayout = attachment.first->getLayout();

			VkAttachmentReference currentAttachmentRef{};
			currentAttachmentRef.attachment = static_cast<uint32_t>(i);
			currentAttachmentRef.layout     = vzt::toVulkan(attachmentLayout);

			m_colorRefs.emplace_back(currentAttachmentRef);
		}

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = static_cast<uint32_t>(m_colorRefs.size());
		depthAttachmentRef.layout     = vzt::toVulkan(depthAttachment.first->getLayout());

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount    = static_cast<uint32_t>(m_colorRefs.size());
		subpass.pColorAttachments       = m_colorRefs.data();
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		m_subpassDependencies = {{VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT,
		                          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		                          VK_DEPENDENCY_BY_REGION_BIT},
		                         {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		                          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		                          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		                          VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT}};

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments    = attachmentDescriptions.data();
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(m_subpassDependencies.size());
		renderPassInfo.pDependencies   = m_subpassDependencies.data();

		if (vkCreateRenderPass(m_device->vkHandle(), &renderPassInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
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
			vkDestroyRenderPass(m_device->vkHandle(), m_vkHandle, nullptr);
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
		for (const auto& m_colorRef : m_colorRefs)
		{
			clearValues[m_colorRef.attachment].color = {0.0f, 0.0f, 0.0f, 1.0f};
		}
		clearValues[m_depthRef.attachment].depthStencil = {1.0f, 0};

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues    = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::unbind(VkCommandBuffer commandBuffer) const { vkCmdEndRenderPass(commandBuffer); }
} // namespace vzt
