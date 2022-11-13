#include "vzt/Image.hpp"

namespace vzt
{
    inline Extent3D    Image::getSize() const { return m_size; }
    inline ImageUsage  Image::getUsage() const { return m_usage; }
    inline Format      Image::getFormat() const { return m_format; }
    inline uint32_t    Image::getMipLevels() const { return m_mipLevels; }
    inline ImageLayout Image::getLayout() const { return m_layout; }
    inline SampleCount Image::getSampleCount() const { return m_sampleCount; }
    inline ImageType   Image::getImageType() const { return m_type; }
    inline SharingMode Image::getSharingMode() const { return m_sharingMode; }
    inline VkImage     Image::getHandle() const { return m_handle; }

    inline ImageAspect ImageView::getAspect() const { return m_aspect; }
    inline Format      ImageView::getFormat() const { return m_format; }
    inline View<Image> ImageView::getImage() const { return m_image; }
    inline VkImageView ImageView::getHandle() const { return m_handle; }
} // namespace vzt
