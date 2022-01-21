#ifndef VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP

#include <vulkan/vulkan.h>

namespace vzt
{
	class Device;
	class FrameBuffer;

	enum class LoadOperation : uint32_t
	{
		Load     = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD,
		Clear    = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
		DontCare = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE
	};

	enum class ImageLayout : uint32_t
	{
		Undefined                     = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
		General                       = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL,
		ColorAttachmentOptimal        = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		DepthStencilAttachmentOptimal = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		DepthStencilReadOnlyOptimal   = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		ShaderReadOnlyOptimal         = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		TransferSrcOptimal            = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		TransferdstOptimal            = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		Preinitialized                = VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED,
		// Provided by VK_VERSION_1_1
		DepthReadOnlyStencilAttachmentOptimal =
		    VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
		// Provided by VK_VERSION_1_1
		DepthAttachmentstencilReadOnlyOptimal =
		    VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
		// Provided by VK_VERSION_1_2
		DepthAttachmentOptimal = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		// Provided by VK_VERSION_1_2
		DepthReadOnlyOptimal = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
		// Provided by VK_VERSION_1_2
		StencilAttachmentOptimal = VkImageLayout::VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
		// Provided by VK_VERSION_1_2
		StencilReadOnlyOptimal = VkImageLayout::VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
		// Provided by VK_KHR_swapchain
		LayoutPresentSrcKHR = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	class RenderPass
	{
	  public:
		RenderPass(vzt::Device* device, VkFormat colorImageFormat);

		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;

		RenderPass(RenderPass&& other) noexcept;
		RenderPass& operator=(RenderPass&& other) noexcept;

		VkRenderPass VkHandle() const { return m_vkHandle; }

		~RenderPass();

		void Bind(VkCommandBuffer commandBuffer, const vzt::FrameBuffer* const frameBuffer) const;
		void Unbind(VkCommandBuffer commandBuffer) const;

	  private:
		VkRenderPass m_vkHandle = VK_NULL_HANDLE;
		vzt::Device* m_device   = nullptr;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP
