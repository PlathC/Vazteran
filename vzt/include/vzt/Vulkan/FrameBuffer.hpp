#ifndef VZT_VULKAN_FRAMEBUFFER_HPP
#define VZT_VULKAN_FRAMEBUFFER_HPP

#include "vzt/Vulkan/Image.hpp"

namespace vzt
{
    class Device;
    class RenderPass;

    class FrameBuffer
    {
      public:
        FrameBuffer(View<Device> device, Extent2D size);

        FrameBuffer(const FrameBuffer&)            = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        FrameBuffer(FrameBuffer&&) noexcept;
        FrameBuffer& operator=(FrameBuffer&&) noexcept;

        ~FrameBuffer();

        void addAttachment(ImageView attachment);
        void compile(View<RenderPass> renderPass);

        inline VkFramebuffer getHandle() const;
        inline Extent2D      getSize() const;

      private:
        VkFramebuffer m_handle = VK_NULL_HANDLE;
        View<Device>  m_device;
        Extent2D      m_size;

        std::vector<ImageView> m_attachments;
    };
} // namespace vzt

#include "vzt/Vulkan/FrameBuffer.inl"

#endif // VZT_VULKAN_FRAMEBUFFER_HPP
