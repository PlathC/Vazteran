#ifndef VAZTERAN_FRAMEWORK_VULKAN_RENDERPASS_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_RENDERPASS_HPP

#include <vulkan/vulkan.h>

namespace vzt
{
	class Device;
	class FrameBuffer;

	class RenderPass
	{
	  public:
		RenderPass(vzt::Device* device, vzt::Size2D<uint32_t> size, VkFormat colorImageFormat);

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

		vzt::Size2D<uint32_t> m_size = {};
	};
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_RENDERPASS_HPP
