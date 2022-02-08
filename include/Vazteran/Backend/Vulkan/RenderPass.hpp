#ifndef VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP

#include <vulkan/vulkan.h>

namespace vzt
{
	class Device;
	class FrameBuffer;

	enum class PipelineBindPoint : uint32_t
	{
		Graphics             = VK_PIPELINE_BIND_POINT_GRAPHICS,
		Compute              = VK_PIPELINE_BIND_POINT_COMPUTE,
		RaytracingKHR        = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		SubpassShadingHuawei = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI,
		RaytracingNV         = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
	};

	class RenderPass
	{
	  public:
		RenderPass(vzt::Device* device, std::size_t subpassCount, std::vector<VkSubpassDependency>&& subpasses,
		           const std::vector<vzt::Attachment*>& attachments);

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

		std::vector<VkSubpassDependency>  m_subpassDependencies;
		std::vector<VkSubpassDescription> m_subpassDescriptions;

		std::vector<VkAttachmentReference> m_colorRefs;
		VkAttachmentReference              m_depthRef;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP
