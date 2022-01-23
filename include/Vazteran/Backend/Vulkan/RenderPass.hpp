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

	class RenderPass
	{
	  public:
		RenderPass(vzt::Device* device, vzt::Format colorImageFormat);

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
