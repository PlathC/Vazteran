#ifndef VAZTERAN_IMAGEUTILS_HPP
#define VAZTERAN_IMAGEUTILS_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Data/Image.hpp"

namespace vzt {
    class LogicalDevice;

    class ImageView {
    public:
        // TODO: add format to vzt::Image
        ImageView(vzt::LogicalDevice* logicalDevice, vzt::Image image, VkFormat format = VK_FORMAT_B8G8R8A8_SRGB);
        ImageView(LogicalDevice* logicalDevice, VkImageView vkHandle, VkImage vkImage, VkDeviceMemory deviceMemory);
        ImageView(LogicalDevice* logicalDevice, Size2D size, VkFormat format, VkImageUsageFlags usage,
                  VkImageAspectFlags aspectFlags, VkImageLayout layout);

        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;

        ImageView(ImageView&& original) noexcept;
        ImageView& operator=(ImageView&& original) noexcept;

        VkImageView VkHandle() const { return m_vkHandle; }

        ~ImageView();

    private:
        vzt::LogicalDevice* m_logicalDevice;

        VkImage m_vkImage = VK_NULL_HANDLE;
        VkDeviceMemory m_deviceMemory;
        VkImageView m_vkHandle = VK_NULL_HANDLE;
    };

    class Sampler {
    public:
        explicit Sampler(vzt::LogicalDevice* logicalDevice);

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        Sampler(Sampler&& other) noexcept;
        Sampler& operator=(Sampler&& other) noexcept;

        VkSampler VkHandle() const { return m_vkHandle; }

        ~Sampler();

    private:
        vzt::LogicalDevice* m_logicalDevice;
        VkSampler m_vkHandle;
    };

    struct ImageHandler {
        ImageView imageView;
        Sampler sampler;
    };
}

#endif //VAZTERAN_IMAGEUTILS_HPP
