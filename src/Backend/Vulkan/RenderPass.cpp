#include <array>
#include <stdexcept>

#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"

namespace vzt
{
	VkAttachmentDescription toVulkan(const AttachmentPassConfiguration& configuration)
	{
		VkAttachmentDescription description{};
		description.initialLayout  = vzt::toVulkan(configuration.second.initialLayout);
		description.finalLayout    = vzt::toVulkan(configuration.second.finalLayout);
		description.stencilLoadOp  = vzt::toVulkan(configuration.second.stencilLoapOp);
		description.stencilStoreOp = vzt::toVulkan(configuration.second.stencilStoreOp);
		description.loadOp         = vzt::toVulkan(configuration.second.loadOp);
		description.storeOp        = vzt::toVulkan(configuration.second.storeOp);
		description.format         = vzt::toVulkan(configuration.first->getFormat());
		description.samples        = vzt::toVulkan(configuration.first->getSampleCount());

		return description;
	}
	VkAttachmentDescription toVulkan(const DepthAttachmentPassConfiguration& configuration)
	{
		VkAttachmentDescription description{};
		description.initialLayout  = vzt::toVulkan(configuration.second.initialLayout);
		description.finalLayout    = vzt::toVulkan(configuration.second.finalLayout);
		description.stencilLoadOp  = vzt::toVulkan(configuration.second.stencilLoapOp);
		description.stencilStoreOp = vzt::toVulkan(configuration.second.stencilStoreOp);
		description.loadOp         = vzt::toVulkan(configuration.second.loadOp);
		description.storeOp        = vzt::toVulkan(configuration.second.storeOp);
		description.format         = vzt::toVulkan(configuration.first->getFormat());
		description.samples        = vzt::toVulkan(configuration.first->getSampleCount());

		return description;
	}

	RenderPass::RenderPass(const vzt::Device* device, const RenderPassConfiguration& configuration) : m_device(device)
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		attachmentDescriptions.reserve(configuration.colorAttachments.size());

		std::vector<VkAttachmentReference> colorRefs{};
		colorRefs.resize(configuration.colorAttachments.size());
		m_clearValues.resize(configuration.colorAttachments.size() + 1);
		for (std::size_t i = 0; i < configuration.colorAttachments.size(); i++)
		{
			const auto& attachment = configuration.colorAttachments[i];
			attachmentDescriptions.emplace_back(toVulkan(attachment));

			VkAttachmentReference& currentAttachmentRef = colorRefs[i];
			currentAttachmentRef.attachment             = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
			currentAttachmentRef.layout                 = vzt::toVulkan(attachment.second.usedLayout);

			m_clearValues[i].color = {
			    attachment.second.clearValue.x,
			    attachment.second.clearValue.y,
			    attachment.second.clearValue.z,
			    attachment.second.clearValue.w,
			};
		}

		m_clearValues[attachmentDescriptions.size()].depthStencil = {
		    configuration.depthAttachment.second.clearValue.x,
		    static_cast<uint32_t>(configuration.depthAttachment.second.clearValue.y)};

		attachmentDescriptions.emplace_back(toVulkan(configuration.depthAttachment));

		VkAttachmentReference depthRef{};
		depthRef.attachment = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
		depthRef.layout     = vzt::toVulkan(configuration.depthAttachment.second.usedLayout);

		std::vector<VkAttachmentReference> inputRefs{};
		inputRefs.resize(configuration.inputAttachments.size());
		for (std::size_t i = 0; i < configuration.inputAttachments.size(); i++)
		{
			const auto& attachment = configuration.inputAttachments[i];
			attachmentDescriptions.emplace_back(toVulkan(attachment));

			VkAttachmentReference& currentAttachmentRef = inputRefs[i];
			currentAttachmentRef.attachment             = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
			currentAttachmentRef.layout                 = vzt::toVulkan(attachment.second.usedLayout);
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount    = static_cast<uint32_t>(colorRefs.size());
		subpass.pColorAttachments       = colorRefs.data();
		subpass.pDepthStencilAttachment = &depthRef;
		subpass.inputAttachmentCount    = static_cast<uint32_t>(inputRefs.size());
		subpass.pInputAttachments       = inputRefs.data();

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments    = attachmentDescriptions.data();
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpass;

		std::vector<VkSubpassDependency> subpassDependencies{};
		subpassDependencies.resize(configuration.dependencies.size());
		for (std::size_t i = 0; i < configuration.dependencies.size(); i++)
		{
			subpassDependencies[i].srcSubpass      = configuration.dependencies[i].src;
			subpassDependencies[i].dstSubpass      = configuration.dependencies[i].dst;
			subpassDependencies[i].srcStageMask    = vzt::toVulkan(configuration.dependencies[i].srcStage);
			subpassDependencies[i].srcAccessMask   = vzt::toVulkan(configuration.dependencies[i].srcAccess);
			subpassDependencies[i].dstStageMask    = vzt::toVulkan(configuration.dependencies[i].dstStage);
			subpassDependencies[i].dstAccessMask   = vzt::toVulkan(configuration.dependencies[i].dstAccess);
			subpassDependencies[i].dependencyFlags = vzt::toVulkan(configuration.dependencies[i].dependencyFlags);
		}
		renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
		renderPassInfo.pDependencies   = subpassDependencies.data();

		if (vkCreateRenderPass(m_device->vkHandle(), &renderPassInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass!");
		}
	}

	RenderPass::RenderPass(RenderPass&& other) noexcept
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_device, other.m_device);
		std::swap(m_clearValues, other.m_clearValues);
	}

	RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_device, other.m_device);
		std::swap(m_clearValues, other.m_clearValues);

		return *this;
	}

	RenderPass::~RenderPass()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_device->vkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
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
		renderPassInfo.clearValueCount   = static_cast<uint32_t>(m_clearValues.size());
		renderPassInfo.pClearValues      = m_clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderPass::unbind(VkCommandBuffer commandBuffer) const { vkCmdEndRenderPass(commandBuffer); }
} // namespace vzt
