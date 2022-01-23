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
		ImageView(vzt::Device* device, vzt::Image image, vzt::Format format = vzt::Format::B8G8R8A8SRGB);
		ImageView(vzt::Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage,
		          vzt::ImageAspect aspectFlags, vzt::ImageLayout layout);
		ImageView(vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageAspect aspect);

		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;

		ImageView(ImageView&& original) noexcept;
		ImageView& operator=(ImageView&& original) noexcept;

		VkImageView VkHandle() const { return m_vkHandle; }

		~ImageView();

	  private:
		vzt::Device* m_device;

		VkImage m_vkImage = VK_NULL_HANDLE;
		// VkDeviceMemory m_deviceMemory;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkImageView   m_vkHandle   = VK_NULL_HANDLE;
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
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_IMAGEUTILS_HPP
