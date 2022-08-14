#ifndef VAZTERAN_VULKAN_IMAGEUTILS_HPP
#define VAZTERAN_VULKAN_IMAGEUTILS_HPP

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
		ImageView() = default;
		ImageView(const Device* device, Image image, vzt::Format format = Format::B8G8R8A8SRGB,
		          ImageLayout layout = ImageLayout::ShaderReadOnlyOptimal);
		ImageView(const Device* device, Uvec2 size, Format format, ImageUsage usage, ImageAspect aspectFlags,
		          vzt::ImageLayout layout);
		ImageView(const Device* device, VkImage image, Format format, ImageAspect aspect);

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
		const Device* m_device;

		bool          m_isProprietary = false;
		VkImage       m_vkImage       = VK_NULL_HANDLE;
		VmaAllocation m_allocation    = VK_NULL_HANDLE;
		VkImageView   m_vkHandle      = VK_NULL_HANDLE;

		ImageLayout m_layout = vzt::ImageLayout::Undefined;
		Format      m_format;
	};

	struct SamplerSettings
	{
		Filter      filter      = vzt::Filter::Linear;
		AddressMode addressMode = vzt::AddressMode::Repeat;
		MipmapMode  mipmapMode  = vzt::MipmapMode::Linear;
		BorderColor borderColor = vzt::BorderColor::IntOpaqueBlack;
	};

	class Sampler
	{
	  public:
		Sampler() = default;
		Sampler(const vzt::Device* logicalDevice, const SamplerSettings& samplerSettings = {});

		Sampler(const Sampler&)            = delete;
		Sampler& operator=(const Sampler&) = delete;

		Sampler(Sampler&& other) noexcept;
		Sampler& operator=(Sampler&& other) noexcept;

		VkSampler vkHandle() const { return m_vkHandle; }

		~Sampler();

	  private:
		const Device* m_device;
		VkSampler     m_vkHandle = VK_NULL_HANDLE;
	};

	class Texture
	{
	  public:
		Texture() = default;
		Texture(const Device* device, const ImageView* imageView, SamplerSettings samplerSettings = {});

		const ImageView* getView() const { return m_imageView; }
		const Sampler*   getSampler() const { return &m_sampler; }

		const Format format() const { return m_imageView->format(); }

	  private:
		Sampler          m_sampler;
		const ImageView* m_imageView;
	};

} // namespace vzt

#endif // VAZTERAN_VULKAN_IMAGEUTILS_HPP
