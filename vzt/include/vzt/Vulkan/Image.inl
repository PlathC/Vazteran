#include "vzt/Vulkan/Image.hpp"

namespace vzt
{
    template <class ValueType>
    static inline DeviceImage DeviceImage::fromData(View<Device> device, ImageUsage usage, Format format,
                                                    const Image<ValueType>& image)
    {
        return fromData(device, usage, format, image.width, image.height,
                        {reinterpret_cast<const uint8_t*>(image.data.data()), image.data.size() * sizeof(ValueType)});
    }

    inline Extent3D    DeviceImage::getSize() const { return m_size; }
    inline ImageUsage  DeviceImage::getUsage() const { return m_usage; }
    inline Format      DeviceImage::getFormat() const { return m_format; }
    inline uint32_t    DeviceImage::getMipLevels() const { return m_mipLevels; }
    inline ImageLayout DeviceImage::getLayout() const { return m_layout; }
    inline SampleCount DeviceImage::getSampleCount() const { return m_sampleCount; }
    inline ImageType   DeviceImage::getImageType() const { return m_type; }
    inline SharingMode DeviceImage::getSharingMode() const { return m_sharingMode; }
    inline VkImage     DeviceImage::getHandle() const { return m_handle; }

    inline ImageAspect       ImageView::getAspect() const { return m_aspect; }
    inline Format            ImageView::getFormat() const { return m_format; }
    inline View<DeviceImage> ImageView::getImage() const { return m_image; }
    inline VkImageView       ImageView::getHandle() const { return m_handle; }
} // namespace vzt
