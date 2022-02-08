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

		const vzt::RenderPass* RenderPass() const { return m_renderPass.get(); }

		void Bind(VkCommandBuffer commandBuffer) const;
		void Unbind(VkCommandBuffer commandBuffer) const;

		vzt::Size2D<uint32_t> Size() const { return m_size; }
		VkFramebuffer         VkHandle() const { return m_vkHandle; }

	  private:
		vzt::Device*  m_device;
		VkFramebuffer m_vkHandle = VK_NULL_HANDLE;

		vzt::Size2D<uint32_t> m_size;

		std::vector<vzt::Attachment>     m_attachments;
		std::unique_ptr<vzt::RenderPass> m_renderPass;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP
