#include <array>
#include <stdexcept>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	RenderPass::RenderPass(vzt::Device* device, vzt::Format colorImageFormat) : m_device(device)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format  = static_cast<VkFormat>(vzt::ToUnderlying(colorImageFormat));
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp  = static_cast<VkAttachmentLoadOp>(vzt::ToUnderlying(vzt::LoadOperation::Clear));
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp =
		    static_cast<VkAttachmentLoadOp>(vzt::ToUnderlying(vzt::LoadOperation::DontCare));
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout  = static_cast<VkImageLayout>(vzt::ToUnderlying(vzt::ImageLayout::Undefined));
		colorAttachment.finalLayout    = static_cast<VkImageLayout>(vzt::ToUnderlying(vzt::ImageLayout::PresentSrcKHR));

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format =
		    static_cast<VkFormat>(vzt::ToUnderlying(m_device->ChosenPhysicalDevice()->FindDepthFormat()));
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp  = static_cast<VkAttachmentLoadOp>(vzt::ToUnderlying(vzt::LoadOperation::Clear));
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp =
		    static_cast<VkAttachmentLoadOp>(vzt::ToUnderlying(vzt::LoadOperation::DontCare));
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout  = static_cast<VkImageLayout>(vzt::ToUnderlying(vzt::ImageLayout::Undefined));
		depthAttachment.finalLayout =
		    static_cast<VkImageLayout>(vzt::ToUnderlying(vzt::ImageLayout::DepthStencilAttachmentOptimal));

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount    = 1;
		subpass.pColorAttachments       = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask =
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask =
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
		VkRenderPassCreateInfo                 renderPassInfo{};
		renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments    = attachments.data();
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies   = &dependency;

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

	void RenderPass::Bind(VkCommandBuffer commandBuffer, const vzt::FrameBuffer* const frameBuffer) const
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass        = m_vkHandle;
		renderPassInfo.framebuffer       = frameBuffer->VkHandle();
		renderPassInfo.renderArea.offset = {0, 0};

		const auto fbSize                = frameBuffer->Size();
		renderPassInfo.renderArea.extent = VkExtent2D{fbSize.width, fbSize.height};

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color           = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil    = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues    = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::Unbind(VkCommandBuffer commandBuffer) const { vkCmdEndRenderPass(commandBuffer); }
} // namespace vzt
