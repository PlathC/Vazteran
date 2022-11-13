#include "vzt/Attachment.hpp"

namespace vzt
{
    Attachment::Attachment(View<Device> device, Extent2D size, Format format, ImageUsage usage)
        : m_device(device), m_format(format), m_sampleCount(SampleCount::Sample1)
    {
        vzt::ImageAspect aspect;
        if ((usage & vzt::ImageUsage::ColorAttachment) == vzt::ImageUsage::ColorAttachment)
        {
            aspect   = vzt::ImageAspect::Color;
            m_layout = vzt::ImageLayout::ColorAttachmentOptimal;
        }
        else if ((usage & vzt::ImageUsage::DepthStencilAttachment) == vzt::ImageUsage::DepthStencilAttachment)
        {
            aspect   = vzt::ImageAspect::Depth;
            m_layout = vzt::ImageLayout::DepthStencilAttachmentOptimal;
        }
    }

} // namespace vzt
