#ifndef VAZTERAN_BACKEND_VULKAN_IMAGEUTILS_HPP
#define VAZTERAN_BACKEND_VULKAN_IMAGEUTILS_HPP

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/ImageTypes.hpp"
#include "Vazteran/Data/Image.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	class Device;

	class ImageView
	{
	  public:
		ImageView(const vzt::Device* device, vzt::Image image, vzt::Format format = vzt::Format::B8G8R8A8SRGB,
		          vzt::ImageLayout layout = vzt::ImageLayout::ShaderReadOnlyOptimal);
		ImageView(const vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage,
		          vzt::ImageAspect aspectFlags, vzt::ImageLayout layout);
		ImageView(const vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageAspect aspect);

		ImageView(const ImageView&)            = delete;
		ImageView& operator=(const ImageView&) = delete;

		ImageView(ImageView&& original) noexcept;
		ImageView& operator=(ImageView&& original) noexcept;

		vzt::ImageLayout layout() const { return m_layout; }
		vzt::Format      format() const { return m_format; }

		VkImage     image() const { return m_vkImage; }
		VkImageView vkHandle() const { return m_vkHandle; }

		~ImageView();

	  private:
		const vzt::Device* m_device;

		VkImage       m_vkImage    = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkImageView   m_vkHandle   = VK_NULL_HANDLE;

		vzt::ImageLayout m_layout = vzt::ImageLayout::Undefined;
		vzt::Format      m_format;
	};

	struct SamplerSettings
	{
		vzt::Filter      filter      = vzt::Filter::Linear;
		vzt::AddressMode addressMode = vzt::AddressMode::Repeat;
		vzt::MipmapMode  mipmapMode  = vzt::MipmapMode::Linear;
		vzt::BorderColor borderColor = vzt::BorderColor::IntOpaqueBlack;
	};

	class Sampler
	{
	  public:
		explicit Sampler(const vzt::Device* logicalDevice, const SamplerSettings& samplerSettings = {});

		Sampler(const Sampler&)            = delete;
		Sampler& operator=(const Sampler&) = delete;

		Sampler(Sampler&& other) noexcept;
		Sampler& operator=(Sampler&& other) noexcept;

		VkSampler vkHandle() const { return m_vkHandle; }

		~Sampler();

	  private:
		const vzt::Device* m_device;
		VkSampler          m_vkHandle;
	};

	class Texture
	{
	  public:
		Texture(const vzt::Device* device, const vzt::ImageView* imageView, vzt::SamplerSettings samplerSettings = {});

		const vzt::ImageView* getView() const { return m_imageView; }
		const vzt::Sampler*   getSampler() const { return &m_sampler; }

		const vzt::Format format() const { return m_imageView->format(); }

	  private:
		vzt::Sampler          m_sampler;
		const vzt::ImageView* m_imageView;
	};

} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_IMAGEUTILS_HPP
