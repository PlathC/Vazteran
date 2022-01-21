#ifndef VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP
#define VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP

#include <vector>

#include "Vazteran/Core/Math.hpp"

namespace vzt
{
	class Device;
	class ImageView;
	class RenderPass;
	class SwapChain;

	class FrameBuffer
	{
	  public:
		FrameBuffer(vzt::Device* device, const RenderPass* const renderPass, VkImage target,
		            const vzt::ImageView* const depthImage, VkFormat imageFormat, vzt::Size2D<uint32_t> size);

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;

		FrameBuffer(FrameBuffer&&) noexcept;
		FrameBuffer& operator=(FrameBuffer&&) noexcept;

		~FrameBuffer();

		vzt::Size2D<uint32_t> Size() const { return m_size; }

		VkFramebuffer VkHandle() const { return m_vkHandle; }

	  private:
		vzt::Device*  m_device;
		VkFramebuffer m_vkHandle = VK_NULL_HANDLE;

		vzt::Size2D<uint32_t> m_size;
		VkImage               m_target;
		VkImageView           m_imageView = VK_NULL_HANDLE;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_FRAMEBUFFER_HPP
