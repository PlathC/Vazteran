#include "vzt/Vulkan/Texture.hpp"

#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    Sampler::Sampler(View<Device> device, SamplerBuilder builder)
        : m_device(device), m_filter(builder.filter), m_addressMode(builder.addressMode),
          m_mipmapMode(builder.mipmapMode), m_borderColor(builder.borderColor)
    {
        const PhysicalDevice             hardware   = m_device->getHardware();
        const VkPhysicalDeviceProperties properties = hardware.getProperties();

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = toVulkan(m_filter);
        samplerInfo.minFilter               = toVulkan(m_filter);
        samplerInfo.addressModeU            = toVulkan(m_addressMode);
        samplerInfo.addressModeV            = toVulkan(m_addressMode);
        samplerInfo.addressModeW            = toVulkan(m_addressMode);
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = toVulkan(m_borderColor);
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = toVulkan(m_mipmapMode);

        vkCheck(vkCreateSampler(m_device->getHandle(), &samplerInfo, nullptr, &m_handle),
                "Failed to create texture sampler!");
    }

    Sampler::Sampler(Sampler&& other) noexcept
        : m_device(std::move(other.m_device)), m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE)),
          m_filter(std::move(other.m_filter)), m_addressMode(std::move(other.m_addressMode)),
          m_mipmapMode(std::move(other.m_mipmapMode)), m_borderColor(std::move(other.m_borderColor))
    {
    }

    Sampler& Sampler::operator=(Sampler&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_filter, other.m_filter);
        std::swap(m_addressMode, other.m_addressMode);
        std::swap(m_mipmapMode, other.m_mipmapMode);
        std::swap(m_borderColor, other.m_borderColor);

        return *this;
    }

    Sampler::~Sampler()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        vkDestroySampler(m_device->getHandle(), m_handle, nullptr);
    }

    Texture::Texture(View<Device> device, View<DeviceImage> image, SamplerBuilder samplerSettings)
        : m_sampler(device, samplerSettings),
          m_imageView(device, image,
                      image->getUsage() == ImageUsage::DepthStencilAttachment ? ImageAspect::Depth : ImageAspect::Color)
    {
    }

    Texture::Texture(Texture&& other) noexcept
        : m_sampler(std::move(other.m_sampler)), m_imageView(std::move(other.m_imageView))
    {
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        std::swap(m_sampler, other.m_sampler);
        std::swap(m_imageView, other.m_imageView);

        return *this;
    }

} // namespace vzt
