#include "vzt/Texture.hpp"

namespace vzt
{
    inline Filter      Sampler::getFilter() const { return m_filter; }
    inline AddressMode Sampler::getAddressMode() const { return m_addressMode; }
    inline MipmapMode  Sampler::getMipmapMode() const { return m_mipmapMode; }
    inline BorderColor Sampler::getBorderColor() const { return m_borderColor; }
    inline VkSampler   Sampler::getHandle() const { return m_handle; }
} // namespace vzt
