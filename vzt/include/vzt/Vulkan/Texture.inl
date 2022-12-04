#include "vzt/Vulkan/Texture.hpp"

namespace vzt
{
    inline Filter      Sampler::getFilter() const { return m_filter; }
    inline AddressMode Sampler::getAddressMode() const { return m_addressMode; }
    inline MipmapMode  Sampler::getMipmapMode() const { return m_mipmapMode; }
    inline BorderColor Sampler::getBorderColor() const { return m_borderColor; }
    inline VkSampler   Sampler::getHandle() const { return m_handle; }

    inline View<ImageView> Texture::getView() const { return m_imageView; }
    inline const Sampler&  Texture::getSampler() const { return m_sampler; }
} // namespace vzt
