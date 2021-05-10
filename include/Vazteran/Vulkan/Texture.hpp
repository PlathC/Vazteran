#ifndef VAZTERAN_TEXTURE_HPP
#define VAZTERAN_TEXTURE_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Data/Image.hpp"

namespace vzt {
    class LogicalDevice;

    class ImageView {
    public:
        ImageView(vzt::LogicalDevice* logicalDevice, vzt::Image image);

        ImageView(ImageView&) = delete;
        ImageView& operator=(ImageView&) = delete;

        ImageView(ImageView&& original) noexcept ;
        ImageView& operator=(ImageView&& original) noexcept;

        VkImageView VkHandle() const { return m_vkHandle; }

        ~ImageView();
    private:
        vzt::LogicalDevice* m_logicalDevice;

        VkImage m_vkImage = VK_NULL_HANDLE;
        VkDeviceMemory m_deviceMemory;
        VkImageView m_vkHandle = VK_NULL_HANDLE;

        vzt::Image m_image;
    };

    class Sampler {
    public:
        explicit Sampler(vzt::LogicalDevice* logicalDevice);

        Sampler(Sampler&) = delete;
        Sampler& operator=(Sampler&) = delete;

        Sampler(Sampler&& other) noexcept;
        Sampler& operator=(Sampler&& other) noexcept;

        VkSampler VkHandle() const { return m_vkHandle; }

        ~Sampler();

    private:
        vzt::LogicalDevice* m_logicalDevice;
        VkSampler m_vkHandle;
    };

    struct TextureHandler {
        std::unique_ptr<ImageView> imageView;
        std::unique_ptr<Sampler> sampler;
    };
}

#endif //VAZTERAN_TEXTURE_HPP
