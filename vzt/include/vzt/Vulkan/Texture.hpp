#ifndef VZT_VULKAN_TEXTURE_HPP
#define VZT_VULKAN_TEXTURE_HPP

#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"
#include "vzt/Vulkan/Image.hpp"

namespace vzt
{
    class Device;

    enum class Filter : uint32_t
    {
        Nearest  = VK_FILTER_NEAREST,
        Linear   = VK_FILTER_LINEAR,
        CubicImg = VK_FILTER_CUBIC_IMG,
        CubicExt = VK_FILTER_CUBIC_EXT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(Filter, VkFilter)

    enum class AddressMode : uint32_t
    {
        Repeat               = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        MirroredRepeat       = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        ClampToEdge          = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        ClampToBorder        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        MirrorClampToEdge    = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
        MirrorClampToEdgeKHR = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(AddressMode, VkSamplerAddressMode)

    enum class MipmapMode : uint32_t
    {
        Nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        Linear  = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(MipmapMode, VkSamplerMipmapMode)

    enum class BorderColor : uint32_t
    {
        FloatTransparentBlack = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        IntTransparentBlack   = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
        FloatOpaqueBlack      = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        IntOpaqueBlack        = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        FloatOpaqueWhite      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        IntOpaqueWhite        = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
        FloatCustomExt        = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT,
        IntCustomExt          = VK_BORDER_COLOR_INT_CUSTOM_EXT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(BorderColor, VkBorderColor)

    struct SamplerBuilder
    {
        Filter      filter      = vzt::Filter::Linear;
        AddressMode addressMode = vzt::AddressMode::Repeat;
        MipmapMode  mipmapMode  = vzt::MipmapMode::Linear;
        BorderColor borderColor = vzt::BorderColor::IntOpaqueBlack;
    };

    class Sampler
    {
      public:
        Sampler() = default;
        Sampler(View<Device> device, SamplerBuilder builder = {});

        Sampler(const Sampler&)            = delete;
        Sampler& operator=(const Sampler&) = delete;

        Sampler(Sampler&& other) noexcept;
        Sampler& operator=(Sampler&& other) noexcept;

        ~Sampler();

        inline Filter      getFilter() const;
        inline AddressMode getAddressMode() const;
        inline MipmapMode  getMipmapMode() const;
        inline BorderColor getBorderColor() const;
        inline VkSampler   getHandle() const;

      private:
        View<Device> m_device;
        VkSampler    m_handle = VK_NULL_HANDLE;
        Filter       m_filter;
        AddressMode  m_addressMode;
        MipmapMode   m_mipmapMode;
        BorderColor  m_borderColor;
    };

    class Texture
    {
      public:
        Texture(View<Device> device, View<Image> image, SamplerBuilder samplerSettings = {});

        Texture(const Texture&)            = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        ~Texture() = default;

        inline View<ImageView> getView() const;
        inline const Sampler&  getSampler() const;

      private:
        Sampler   m_sampler;
        ImageView m_imageView;
    };

} // namespace vzt

#include "vzt/Vulkan/Texture.inl"

#endif // VZT_VULKAN_TEXTURE_HPP
