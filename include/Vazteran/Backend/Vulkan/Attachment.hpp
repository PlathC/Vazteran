#ifndef VAZTERAN_BACKEND_VULKAN_ATTACHMENT_HPP
#define VAZTERAN_BACKEND_VULKAN_ATTACHMENT_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Core/Math.hpp"

namespace vzt
{
	class Device;

	class Attachment
	{
	  public:
		Attachment(vzt::Device* device, const vzt::Image& image, vzt::Format format);
		Attachment(vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage);
		Attachment(vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageAspect aspect);
		~Attachment() = default;

		Attachment(const Attachment&) = delete;
		Attachment& operator=(const Attachment&) = delete;

		Attachment(Attachment&&) = default;
		Attachment& operator=(Attachment&&) = default;

		const vzt::ImageView* View() const { return m_imageView.get(); }
		const vzt::Sampler*   Sampler() const { return &m_sampler; }
		const vzt::Format     Format() const { return m_format; }

	  private:
		vzt::Device*                    m_device;
		vzt::Format                     m_format;
		vzt::Sampler                    m_sampler;
		std::unique_ptr<vzt::ImageView> m_imageView;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_ATTACHMENT_HPP
