#include "Vazteran/Backend/Vulkan/Attachment.hpp"

namespace vzt
{
	Attachment::Attachment(vzt::Device* device, const vzt::Image& image, vzt::Format format)
	    : m_device(device), m_format(format), m_sampler(device),
	      m_imageView(std::make_unique<vzt::ImageView>(m_device, image, format))
	{
	}

	Attachment::Attachment(vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage)
	    : m_device(device), m_format(format), m_sampler(device)
	{
		vzt::ImageAspect aspect;
		vzt::ImageLayout layout;
		if ((usage & vzt::ImageUsage::ColorAttachment) == vzt::ImageUsage::ColorAttachment)
		{
			aspect = vzt::ImageAspect::Color;
			layout = vzt::ImageLayout::ColorAttachmentOptimal;
		}
		else if ((usage & vzt::ImageUsage::DepthStencilAttachment) == vzt::ImageUsage::DepthStencilAttachment)
		{
			aspect = vzt::ImageAspect::Depth;
			layout = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		}

		m_imageView = std::make_unique<vzt::ImageView>(m_device, size, format, usage, aspect, layout);
	}

	Attachment::Attachment(vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageAspect aspect)
	    : m_device(device), m_format(format), m_sampler(device)
	{
		m_imageView = std::make_unique<vzt::ImageView>(m_device, image, format, aspect);
	}
} // namespace vzt
