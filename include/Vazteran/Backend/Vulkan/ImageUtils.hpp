#ifndef VAZTERAN_BACKEND_VULKAN_IMAGEUTILS_HPP
#define VAZTERAN_BACKEND_VULKAN_IMAGEUTILS_HPP

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/ImageTypes.hpp"
#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Data/Image.hpp"

namespace vzt
{
	class Device;

	class ImageView
	{
	  public:
		ImageView(vzt::Device* device, vzt::Image image, vzt::Format format = vzt::Format::B8G8R8A8SRGB,
		          vzt::ImageLayout layout = vzt::ImageLayout::ShaderReadOnlyOptimal);
		ImageView(vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage,
		          vzt::ImageAspect aspectFlags, vzt::ImageLayout layout);
		ImageView(vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageAspect aspect);

		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;

		ImageView(ImageView&& original) noexcept;
		ImageView& operator=(ImageView&& original) noexcept;

		vzt::ImageLayout Layout() const { return m_layout; }
		vzt::Format      Format() const { return m_format; }
		VkImageView      VkHandle() const { return m_vkHandle; }

		~ImageView();

	  private:
		vzt::Device* m_device;

		VkImage       m_vkImage    = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkImageView   m_vkHandle   = VK_NULL_HANDLE;

		vzt::ImageLayout m_layout = vzt::ImageLayout::Undefined;
		vzt::Format      m_format;
	};

	class Sampler
	{
	  public:
		explicit Sampler(vzt::Device* logicalDevice);

		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;

		Sampler(Sampler&& other) noexcept;
		Sampler& operator=(Sampler&& other) noexcept;

		VkSampler VkHandle() const { return m_vkHandle; }

		~Sampler();

	  private:
		vzt::Device* m_logicalDevice;
		VkSampler    m_vkHandle;
	};

	class Texture
	{
	  public:
		Texture(vzt::Device* device, const vzt::Image& image, vzt::Format format);

		const vzt::ImageView* View() const { return m_imageView.get(); }
		const vzt::Sampler*   Sampler() const { return &m_sampler; }
		const vzt::Format     Format() const { return m_format; }

	  private:
		vzt::Format                     m_format;
		vzt::Sampler                    m_sampler;
		std::unique_ptr<vzt::ImageView> m_imageView;
	};

} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_IMAGEUTILS_HPP
