#include <iostream>
#include <utility>

#include "Vazteran/Utils.hpp"
#include "Vazteran/Vulkan/ImageUtils.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"


namespace vzt {
    ImageView::ImageView(LogicalDevice* logicalDevice, vzt::Image image, VkFormat format):
            m_logicalDevice(logicalDevice) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VkDeviceSize imageSize = image.Width() * image.Height() * image.Channels();
        m_logicalDevice->CreateBuffer(stagingBuffer, stagingBufferMemory, imageSize,
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        auto imageData = image.Data();
        vkMapMemory(m_logicalDevice->VkHandle(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, imageData.data(), static_cast<size_t>(imageSize));
        vkUnmapMemory(m_logicalDevice->VkHandle(), stagingBufferMemory);

        m_logicalDevice->CreateImage(
                m_vkImage, m_deviceMemory, image.Width(), image.Height(),
                format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_logicalDevice->TransitionImageLayout(
                m_vkImage, format,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT
        );

        m_logicalDevice->CopyBufferToImage(
                stagingBuffer, m_vkImage, image.Width(), image.Height()
        );

        m_logicalDevice->TransitionImageLayout(
                m_vkImage, format,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT
        );

        vkDestroyBuffer(m_logicalDevice->VkHandle(), stagingBuffer, nullptr);
        vkFreeMemory(m_logicalDevice->VkHandle(), stagingBufferMemory, nullptr);
        m_vkHandle = m_logicalDevice->CreateImageView(
                m_vkImage, format,
                VK_IMAGE_ASPECT_COLOR_BIT
        );
    }

    ImageView::ImageView(LogicalDevice* logicalDevice, VkImageView vkHandle, VkImage vkImage, VkDeviceMemory deviceMemory):
            m_logicalDevice(logicalDevice), m_vkHandle(vkHandle), m_vkImage(vkImage), m_deviceMemory(deviceMemory) {

    }

    ImageView::ImageView(LogicalDevice* logicalDevice, Size2D<uint32_t> size, VkFormat format, VkImageUsageFlags usage,
                         VkImageAspectFlags aspectFlags, VkImageLayout layout):
            m_logicalDevice(logicalDevice){
        m_logicalDevice->CreateImage(m_vkImage, m_deviceMemory, size.width, size.height, format,
                                     VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_vkHandle = m_logicalDevice->CreateImageView(m_vkImage, format, aspectFlags);
        m_logicalDevice->TransitionImageLayout( m_vkImage, format,VK_IMAGE_LAYOUT_UNDEFINED, layout,
                                                aspectFlags);
    }

    ImageView::ImageView(ImageView&& original) noexcept :
            m_logicalDevice(original.m_logicalDevice) {
        m_vkImage = std::exchange(original.m_vkImage, static_cast<decltype(m_vkImage)>(VK_NULL_HANDLE));
        m_vkHandle = std::exchange(original.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
        m_deviceMemory = std::exchange(original.m_deviceMemory, static_cast<decltype(m_deviceMemory)>(VK_NULL_HANDLE));
    }

    ImageView& ImageView::operator=(ImageView&& original) noexcept {
        m_logicalDevice = original.m_logicalDevice;
        std::swap(m_vkImage, original.m_vkImage);
        std::swap(m_vkHandle, original.m_vkHandle);
        std::swap(m_deviceMemory, original.m_deviceMemory);

        return *this;
    }

    ImageView::~ImageView() {
        if (m_vkHandle != VK_NULL_HANDLE){
            vkDestroyImageView(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
        }

        if (m_vkImage != VK_NULL_HANDLE) {
            vkDestroyImage(m_logicalDevice->VkHandle(), m_vkImage, nullptr);
        }

        if (m_deviceMemory != VK_NULL_HANDLE) {
            vkFreeMemory(m_logicalDevice->VkHandle(), m_deviceMemory, nullptr);
        }
    }

    Sampler::Sampler(LogicalDevice* logicalDevice) :
            m_logicalDevice(logicalDevice) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_logicalDevice->Parent()->VkHandle(), &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(m_logicalDevice->VkHandle(), &samplerInfo, nullptr, &m_vkHandle) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture sampler!");
        }
    }

    Sampler::Sampler(Sampler&& other) noexcept {
        m_logicalDevice = std::exchange(other.m_logicalDevice, nullptr);
        m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
    }

    Sampler& Sampler::operator=(Sampler&& other) noexcept {
        std::swap(m_logicalDevice, other.m_logicalDevice);
        std::swap(m_vkHandle, other.m_vkHandle);

        return *this;
    }

    Sampler::~Sampler() {
        if (m_vkHandle != VK_NULL_HANDLE) {
            vkDestroySampler(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
        }
    }
}