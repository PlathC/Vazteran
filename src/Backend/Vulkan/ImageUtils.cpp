#include <iostream>
#include <utility>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	ImageView::ImageView(Device* device, vzt::Image image, vzt::Format format, vzt::ImageLayout layout)
	    : m_device(device), m_format(format), m_layout(layout)
	{
		VkBuffer      stagingBuffer   = VK_NULL_HANDLE;
		VmaAllocation stagingBufAlloc = VK_NULL_HANDLE;
		VkDeviceSize  imageSize       = image.Width() * image.Height() * image.Channels();

		stagingBuffer = m_device->CreateBuffer(stagingBufAlloc, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                                       VMA_MEMORY_USAGE_CPU_ONLY);

		void* data;
		auto  imageData = image.Data();
		vmaMapMemory(m_device->AllocatorHandle(), stagingBufAlloc, &data);
		memcpy(data, imageData.data(), static_cast<size_t>(imageSize));
		vmaUnmapMemory(m_device->AllocatorHandle(), stagingBufAlloc);
		m_vkImage =
		    m_device->CreateImage(m_allocation, image.Width(), image.Height(), format, VK_SAMPLE_COUNT_1_BIT,
		                          VK_IMAGE_TILING_OPTIMAL, vzt::ImageUsage::TransferDst | vzt::ImageUsage::Sampled);

		m_device->TransitionImageLayout(m_vkImage, vzt::ImageLayout::Undefined, vzt::ImageLayout::TransferDstOptimal,
		                                vzt::ImageAspect::Color);

		m_device->CopyBufferToImage(stagingBuffer, m_vkImage, image.Width(), image.Height());

		m_device->TransitionImageLayout(m_vkImage, vzt::ImageLayout::TransferDstOptimal, layout,
		                                vzt::ImageAspect::Color);

		vmaDestroyBuffer(m_device->AllocatorHandle(), stagingBuffer, stagingBufAlloc);

		m_vkHandle = m_device->CreateImageView(m_vkImage, format, vzt::ImageAspect::Color);
	}

	ImageView::ImageView(Device* device, vzt::Size2D<uint32_t> size, vzt::Format format, vzt::ImageUsage usage,
	                     vzt::ImageAspect aspectFlags, vzt::ImageLayout layout)
	    : m_device(device), m_format(format), m_layout(layout)
	{

		m_vkImage = m_device->CreateImage(m_allocation, size.width, size.height, format, VK_SAMPLE_COUNT_1_BIT,
		                                  VK_IMAGE_TILING_OPTIMAL, usage);

		m_vkHandle = m_device->CreateImageView(m_vkImage, format, aspectFlags);
	}

	ImageView::ImageView(vzt::Device* device, VkImage image, vzt::Format format, vzt::ImageAspect aspect)
	    : m_device(device), m_format(format)
	{
		m_vkHandle = m_device->CreateImageView(image, format, aspect);
	}

	ImageView::ImageView(ImageView&& original) noexcept : m_device(original.m_device)
	{
		m_vkImage    = std::exchange(original.m_vkImage, static_cast<decltype(m_vkImage)>(VK_NULL_HANDLE));
		m_vkHandle   = std::exchange(original.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		m_allocation = std::exchange(original.m_allocation, static_cast<decltype(m_allocation)>(VK_NULL_HANDLE));
	}

	ImageView& ImageView::operator=(ImageView&& original) noexcept
	{
		m_device = original.m_device;
		std::swap(m_vkImage, original.m_vkImage);
		std::swap(m_vkHandle, original.m_vkHandle);
		std::swap(m_allocation, original.m_allocation);

		return *this;
	}

	ImageView::~ImageView()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_device->VkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}

		if (m_vkImage != VK_NULL_HANDLE)
		{
			vmaDestroyImage(m_device->AllocatorHandle(), m_vkImage, m_allocation);
			m_vkImage = VK_NULL_HANDLE;
		}
	}

	Sampler::Sampler(const vzt::Device* device, const SamplerSettings& samplerSettings) : m_device(device)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_device->ChosenPhysicalDevice()->VkHandle(), &properties);

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

		if (vkCreateSampler(m_device->VkHandle(), &samplerInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
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
			vkDestroySampler(m_device->VkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

	Texture::Texture(const vzt::Device* device, const vzt::ImageView* imageView, vzt::SamplerSettings samplerSettings)
	    : m_sampler(device, samplerSettings), m_imageView(imageView)
	{
	}

} // namespace vzt
