#ifndef VZT_VULKAN_FRAMEBUFFER_HPP
#define VZT_VULKAN_FRAMEBUFFER_HPP

#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/image.hpp"

namespace vzt
{
    class RenderPass;

    class FrameBuffer : public DeviceObject<VkFramebuffer>
    {
      public:
        FrameBuffer(View<Device> device, Extent2D size);

        FrameBuffer(const FrameBuffer&)            = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        FrameBuffer(FrameBuffer&&) noexcept;
        FrameBuffer& operator=(FrameBuffer&&) noexcept;

        ~FrameBuffer() override;

        void addAttachment(ImageView attachment);
        void compile(View<RenderPass> renderPass);

        inline Extent2D getSize() const;

      private:
        Extent2D               m_size;
        std::vector<ImageView> m_attachments;
    };
} // namespace vzt

#include "vzt/vulkan/frame_buffer.inl"

#endif // VZT_VULKAN_FRAMEBUFFER_HPP
