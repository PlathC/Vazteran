#ifndef VAZTERAN_TEXTURE_HPP
#define VAZTERAN_TEXTURE_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Data/Image.hpp"

namespace vzt {
    class LogicalDevice;

    class TextureImage {
    public:
        TextureImage(LogicalDevice* logicalDevice, Image  image);

        TextureImage(TextureImage&) = delete;
        TextureImage& operator=(TextureImage&) = delete;

        TextureImage(TextureImage&& original);
        TextureImage& operator=(TextureImage&& original) noexcept;

        VkImageView ImageView() const { return m_textureImageView; }

        ~TextureImage();
    private:
        LogicalDevice* m_logicalDevice;
        VkImage m_textureImage = VK_NULL_HANDLE;
        VkDeviceMemory m_textureImageMemory;
        VkImageView m_textureImageView = VK_NULL_HANDLE;

        Image m_image;
    };

    class TextureSampler {
    public:
        TextureSampler(LogicalDevice* logicalDevice);

        VkSampler VkHandle() const { return m_vkHandle; }

        ~TextureSampler();
    private:
        LogicalDevice* m_logicalDevice;
        VkSampler m_vkHandle;
    };
}

#endif //VAZTERAN_TEXTURE_HPP
