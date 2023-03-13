#include "vzt/Vulkan/Image.hpp"

namespace vzt
{
    template <class ValueType>
    static inline DeviceImage DeviceImage::fromData(View<Device> device, ImageUsage usage, Format format,
                                                    const Image<ValueType>& image, uint32_t mipLevels,
                                                    ImageLayout layout, SampleCount sampleCount, ImageType type,
                                                    SharingMode sharingMode, ImageTiling tiling, bool mappable)
    {
        return fromData(device, usage, format, image.width, image.height,
                        {reinterpret_cast<const uint8_t*>(image.data.data()), image.data.size() * sizeof(ValueType)},
                        mipLevels, layout, sampleCount, type, sharingMode, tiling, mappable);
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
    inline VkImage       DeviceImage::getHandle() const { return m_handle; }

    inline ImageAspect       ImageView::getAspect() const { return m_aspect; }
    inline Format            ImageView::getFormat() const { return m_format; }
    inline View<DeviceImage> ImageView::getImage() const { return m_image; }
    inline VkImageView       ImageView::getHandle() const { return m_handle; }
} // namespace vzt
