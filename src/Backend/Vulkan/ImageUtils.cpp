#include <iostream>
#include <utility>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Core/Logger.hpp"

namespace vzt
{
	ImageView::ImageView(const Device* device, Image image, Format format, ImageLayout layout)
	    : m_device(device), m_format(format), m_layout(layout), m_isProprietary(true)
	{
		VkBuffer      stagingBuffer   = VK_NULL_HANDLE;
		VmaAllocation stagingBufAlloc = VK_NULL_HANDLE;
		VkDeviceSize  imageSize       = image.width() * image.height() * image.channels();

		stagingBuffer = m_device->createBuffer(stagingBufAlloc, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                                       VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		                                       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		void* data;
		auto  imageData = image.data();
		vmaMapMemory(m_device->getAllocatorHandle(), stagingBufAlloc, &data);
		memcpy(data, imageData.data(), static_cast<size_t>(imageSize));
		vmaUnmapMemory(m_device->getAllocatorHandle(), stagingBufAlloc);
		m_vkImage =
		    m_device->createImage(m_allocation, image.width(), image.height(), format, VK_SAMPLE_COUNT_1_BIT,
		                          VK_IMAGE_TILING_OPTIMAL, vzt::ImageUsage::TransferDst | vzt::ImageUsage::Sampled);

		m_device->transitionImageLayout(m_vkImage, vzt::ImageLayout::Undefined, vzt::ImageLayout::TransferDstOptimal,
		                                vzt::ImageAspect::Color);

		m_device->copyBufferToImage(stagingBuffer, m_vkImage, image.width(), image.height());

		m_device->transitionImageLayout(m_vkImage, vzt::ImageLayout::TransferDstOptimal, layout,
		                                vzt::ImageAspect::Color);

		vmaDestroyBuffer(m_device->getAllocatorHandle(), stagingBuffer, stagingBufAlloc);

		m_vkHandle = m_device->createImageView(m_vkImage, format, vzt::ImageAspect::Color);
	}

	ImageView::ImageView(const Device* device, Uvec2 size, vzt::Format format, vzt::ImageUsage usage,
	                     vzt::ImageAspect aspectFlags, vzt::ImageLayout layout)
	    : m_device(device), m_format(format), m_layout(layout), m_isProprietary(true)
	{
		m_vkImage  = m_device->createImage(m_allocation, size.x, size.y, format, VK_SAMPLE_COUNT_1_BIT,
		                                   VK_IMAGE_TILING_OPTIMAL, usage);
		m_vkHandle = m_device->createImageView(m_vkImage, format, aspectFlags);
	}

	ImageView::ImageView(const vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageAspect aspect)
	    : m_device(device), m_format(format), m_isProprietary(false)
	{
		m_vkHandle = m_device->createImageView(image, format, aspect);
		m_vkImage  = image;
	}

	ImageView::ImageView(ImageView&& original) noexcept : m_device(original.m_device)
	{
		std::swap(m_isProprietary, original.m_isProprietary);
		std::swap(m_format, original.m_format);
		std::swap(m_vkImage, original.m_vkImage);
		std::swap(m_vkHandle, original.m_vkHandle);
		std::swap(m_allocation, original.m_allocation);
	}

	ImageView& ImageView::operator=(ImageView&& original) noexcept
	{
		std::swap(m_device, original.m_device);
		std::swap(m_isProprietary, original.m_isProprietary);
		std::swap(m_format, original.m_format);
		std::swap(m_vkImage, original.m_vkImage);
		std::swap(m_vkHandle, original.m_vkHandle);
		std::swap(m_allocation, original.m_allocation);

		return *this;
	}

	ImageView::~ImageView()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_device->vkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}

		if (m_isProprietary && m_vkImage != VK_NULL_HANDLE)
		{
			vmaDestroyImage(m_device->getAllocatorHandle(), m_vkImage, m_allocation);
			m_vkImage = VK_NULL_HANDLE;
		}
	}

	Sampler::Sampler(const vzt::Device* device, const SamplerSettings& samplerSettings) : m_device(device)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_device->getPhysicalDevice()->vkHandle(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter               = static_cast<VkFilter>(samplerSettings.filter);
		samplerInfo.minFilter               = static_cast<VkFilter>(samplerSettings.filter);
		samplerInfo.addressModeU            = static_cast<VkSamplerAddressMode>(samplerSettings.addressMode);
		samplerInfo.addressModeV            = static_cast<VkSamplerAddressMode>(samplerSettings.addressMode);
		samplerInfo.addressModeW            = static_cast<VkSamplerAddressMode>(samplerSettings.addressMode);
		samplerInfo.anisotropyEnable        = VK_TRUE;
		samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor             = static_cast<VkBorderColor>(samplerSettings.borderColor);
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable           = VK_FALSE;
		samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode              = static_cast<VkSamplerMipmapMode>(samplerSettings.mipmapMode);

		if (vkCreateSampler(m_device->vkHandle(), &samplerInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	Sampler::Sampler(Sampler&& other) noexcept
	{
		m_device   = std::exchange(other.m_device, nullptr);
		m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
	}

	Sampler& Sampler::operator=(Sampler&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);

		return *this;
	}

	Sampler::~Sampler()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroySampler(m_device->vkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

	Texture::Texture(const Device* device, const ImageView* imageView, SamplerSettings samplerSettings)
	    : m_sampler(device, samplerSettings), m_imageView(imageView)
	{
	}

} // namespace vzt
