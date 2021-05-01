#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Vazteran/Vulkan/Data/Texture.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"

#include <iostream>

namespace vzt {
    TextureImage::TextureImage(LogicalDevice* logicalDevice, const fs::path &imagePath):
            m_logicalDevice(logicalDevice) {

        int width, height, channels;
        stbi_uc* pixels = stbi_load(imagePath.string().c_str(),
                                    &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }

        m_width = width;
        m_height = height;
        m_channels = channels;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VkDeviceSize imageSize = m_width * m_height * 4;
        m_logicalDevice->CreateBuffer(stagingBuffer, stagingBufferMemory, imageSize,
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        vkMapMemory(m_logicalDevice->VkHandle(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_logicalDevice->VkHandle(), stagingBufferMemory);

        m_logicalDevice->CreateImage(
                m_textureImage, m_textureImageMemory, m_width, m_height,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_logicalDevice->TransitionImageLayout(
                m_textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT
        );
        m_logicalDevice->CopyBufferToImage(
                stagingBuffer, m_textureImage,
                static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)
        );
        m_logicalDevice->TransitionImageLayout(
                m_textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT
        );

        vkDestroyBuffer(m_logicalDevice->VkHandle(), stagingBuffer, nullptr);
        vkFreeMemory(m_logicalDevice->VkHandle(), stagingBufferMemory, nullptr);

        m_textureImageView = m_logicalDevice->CreateImageView(
                m_textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_ASPECT_COLOR_BIT
        );
    }

    TextureImage::TextureImage(TextureImage&& original):
            m_logicalDevice(original.m_logicalDevice),
            m_textureImageMemory(original.m_textureImageMemory),
            m_width(original.m_width),
            m_height(original.m_height),
            m_channels(original.m_channels) {
        std::swap(m_textureImage, original.m_textureImage);
        std::swap(m_textureImageView, original.m_textureImageView);
    }

    TextureImage& TextureImage::operator=(TextureImage&& original) noexcept {
        m_logicalDevice = original.m_logicalDevice;
        m_textureImageMemory = original.m_textureImageMemory;
        m_width = original.m_width;
        m_height = original.m_height;
        m_channels = original.m_channels;
        std::swap(m_textureImage, original.m_textureImage);
        std::swap(m_textureImageView, original.m_textureImageView);

        return *this;
    }

    TextureImage::~TextureImage() {
        if (m_textureImageView != VK_NULL_HANDLE)
            vkDestroyImageView(m_logicalDevice->VkHandle(), m_textureImageView, nullptr);

        if (m_textureImage != VK_NULL_HANDLE)
            vkDestroyImage(m_logicalDevice->VkHandle(), m_textureImage, nullptr);
        vkFreeMemory(m_logicalDevice->VkHandle(), m_textureImageMemory, nullptr);
    }

    TextureSampler::TextureSampler(LogicalDevice* logicalDevice) :
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
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    TextureSampler::~TextureSampler() {
        vkDestroySampler(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
    }
}