#include "Vazteran/Backend/Vulkan/Attachment.hpp"

namespace vzt
{
	Attachment::Attachment(const vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format,
	                       vzt::ImageUsage usage)
	    : m_device(device), m_format(format)
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

		m_imageView = std::make_unique<vzt::ImageView>(m_device, size, format, usage, aspect, m_layout);
	}

	Attachment::Attachment(const vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageLayout layout,
	                       vzt::ImageAspect aspect)
	    : m_device(device), m_format(format), m_layout(layout), m_sampleCount(vzt::SampleCount::Sample1)
	{
		m_imageView = std::make_unique<vzt::ImageView>(m_device, image, format, aspect);
	}

	vzt::Texture* Attachment::asTexture() const
	{
		if (!m_textureRepresentation)
		{
			vzt::SamplerSettings samplerSettings{vzt::Filter::Nearest, vzt::AddressMode::ClampToEdge,
			                                     vzt::MipmapMode::Linear, vzt::BorderColor::FloatOpaqueWhite};
			m_textureRepresentation = std::make_unique<vzt::Texture>(m_device, m_imageView.get(), samplerSettings);
		}
		return m_textureRepresentation.get();
	}
} // namespace vzt
