#ifndef VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP
#define VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP

#include <vector>

#include "Vazteran/Backend/Vulkan/ImageTypes.hpp"
#include "Vazteran/Core/Math.hpp"

namespace vzt
{
	class Attachment;
	class Device;
	class RenderPass;

	class FrameBuffer
	{
	  public:
		FrameBuffer(vzt::Device* device, std::size_t subpassCount,
		            std::vector<VkSubpassDependency>&& subpassDependencies, std::vector<vzt::Attachment>&& attachments,
		            vzt::Size2D<uint32_t> size);

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;

		FrameBuffer(FrameBuffer&&) noexcept;
		FrameBuffer& operator=(FrameBuffer&&) noexcept;

		~FrameBuffer();

		const vzt::RenderPass* getRenderPass() const { return m_renderPass.get(); }
		vzt::Size2D<uint32_t>  size() const { return m_size; }

		void bind(VkCommandBuffer commandBuffer) const;
		void unbind(VkCommandBuffer commandBuffer) const;

		VkFramebuffer vkHandle() const { return m_vkHandle; }

	  private:
		vzt::Device*  m_device;
		VkFramebuffer m_vkHandle = VK_NULL_HANDLE;

		vzt::Size2D<uint32_t> m_size;

		std::vector<vzt::Attachment>     m_attachments;
		std::unique_ptr<vzt::RenderPass> m_renderPass;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP
