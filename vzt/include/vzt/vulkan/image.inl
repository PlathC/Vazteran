#include "vzt/vulkan/image.hpp"

namespace vzt
{
    template <class ValueType>
    DeviceImage DeviceImage::From(View<Device> device, ImageUsage usage, Format format, uint32_t width, uint32_t height,
                                  const CSpan<ValueType> data, uint32_t mipLevels, ImageLayout layout,
                                  SampleCount sampleCount, ImageType type, SharingMode sharingMode, ImageTiling tiling,
                                  bool mappable)
    {
        return From(device, usage, format, width, height,
                    {reinterpret_cast<const uint8_t*>(data.data), data.size * sizeof(ValueType)}, mipLevels, layout,
                    sampleCount, type, sharingMode, tiling, mappable);
    }

    template <class Type>
    Type* DeviceImage::map()
    {
        return reinterpret_cast<Type*>(map());
    }
    template <class Type>
    const Type* DeviceImage::map() const
    {
        return reinterpret_cast<const Type*>(map());
    }

    inline Extent3D      DeviceImage::getSize() const { return m_size; }
    inline ImageUsage    DeviceImage::getUsage() const { return m_usage; }
    inline Format        DeviceImage::getFormat() const { return m_format; }
    inline uint32_t      DeviceImage::getMipLevels() const { return m_mipLevels; }
    inline ImageLayout   DeviceImage::getLayout() const { return m_layout; }
    inline SampleCount   DeviceImage::getSampleCount() const { return m_sampleCount; }
    inline ImageType     DeviceImage::getImageType() const { return m_type; }
    inline SharingMode   DeviceImage::getSharingMode() const { return m_sharingMode; }
    inline VmaAllocation DeviceImage::getAllocation() const { return m_allocation; }

    inline ImageAspect       ImageView::getAspect() const { return m_aspect; }
    inline Format            ImageView::getFormat() const { return m_format; }
    inline View<DeviceImage> ImageView::getImage() const { return m_image; }
    inline VkImageView       ImageView::getHandle() const { return m_handle; }

    inline Filter      Sampler::getFilter() const { return m_filter; }
    inline AddressMode Sampler::getAddressMode() const { return m_addressMode; }
    inline MipmapMode  Sampler::getMipmapMode() const { return m_mipmapMode; }
    inline BorderColor Sampler::getBorderColor() const { return m_borderColor; }

    inline View<ImageView> Texture::getView() const { return m_imageView; }
    inline const Sampler&  Texture::getSampler() const { return m_sampler; }
} // namespace vzt
