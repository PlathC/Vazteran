#ifndef VAZTERAN_TEXTURE_HPP
#define VAZTERAN_TEXTURE_HPP

#include <filesystem>

namespace fs = std::filesystem;

#include <vulkan/vulkan.h>

namespace vzt {
    class LogicalDevice;

    class TextureImage {
    public:
        TextureImage(LogicalDevice* logicalDevice, const fs::path& imagePath);

        TextureImage(TextureImage&) = delete;
        TextureImage& operator=(TextureImage&) = delete;

        TextureImage(TextureImage&& original);
        TextureImage& operator=(TextureImage&& original) noexcept;

        void TransitionLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        VkImageView ImageView() const { return m_textureImageView; }

        ~TextureImage();
    private:
        LogicalDevice* m_logicalDevice;
        VkImage m_textureImage = VK_NULL_HANDLE;
        VkDeviceMemory m_textureImageMemory;
        VkImageView m_textureImageView = VK_NULL_HANDLE;

        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_channels;
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
