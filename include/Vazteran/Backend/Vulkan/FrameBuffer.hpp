#ifndef VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP
#define VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP

#include <vector>

#include "Vazteran/Backend/Vulkan/ImageTypes.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	class Attachment;
	class Device;
	class ImageView;
	class RenderPass;

	class FrameBuffer
	{
	  public:
		FrameBuffer(const vzt::Device* device, std::unique_ptr<vzt::RenderPass> renderPass, vzt::Size2D<uint32_t> size,
		            const std::vector<const vzt::ImageView*>& extAttachmentViews);

		FrameBuffer(const FrameBuffer&)            = delete;
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
		const vzt::Device*               m_device   = nullptr;
		VkFramebuffer                    m_vkHandle = VK_NULL_HANDLE;
		vzt::Size2D<uint32_t>            m_size{};
		std::unique_ptr<vzt::RenderPass> m_renderPass{};
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP
