#ifndef VZT_ATTACHMENT_HPP
#define VZT_ATTACHMENT_HPP

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Image.hpp"

namespace vzt
{
    class Device;

    class Attachment
    {
      public:
        Attachment(View<Device> device, Extent2D size, Format format, ImageUsage usage, ImageAspect aspect);
        Attachment(View<Device> device, Extent2D size, Format format, ImageUsage usage);
        Attachment(View<Device> device, VkImage image, Format format, ImageLayout layout, ImageAspect aspect);
        ~Attachment();

        Attachment(const Attachment&)            = delete;
        Attachment& operator=(const Attachment&) = delete;

        Attachment(Attachment&&)            = default;
        Attachment& operator=(Attachment&&) = default;

      private:
        View<Device> m_device;
        Format       m_format;
        ImageLayout  m_layout;
        SampleCount  m_sampleCount;
    };

} // namespace vzt

#endif // VZT_ATTACHMENT_HPP
