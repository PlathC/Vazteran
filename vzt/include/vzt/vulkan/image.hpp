#ifndef VZT_VULKAN_IMAGE_HPP
#define VZT_VULKAN_IMAGE_HPP

#include "vzt/core/math.hpp"
#include "vzt/core/type.hpp"
#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/type.hpp"

namespace vzt
{
    struct ImageBuilder
    {
        Extent3D    size;
        ImageUsage  usage;
        Format      format;
        uint32_t    mipLevels   = 1;
        SampleCount sampleCount = SampleCount::Sample1;
        ImageType   type        = ImageType::T2D;
        SharingMode sharingMode = SharingMode::Exclusive;
        ImageTiling tiling      = ImageTiling::Optimal;
        bool        mappable    = false;
    };

    struct SubresourceLayout
    {
        uint64_t offset;
        uint64_t size;
        uint64_t rowPitch;
        uint64_t arrayPitch;
        uint64_t depthPitch;
    };

    class DeviceImage : public DeviceObject<VkImage>
    {
      public:
        template <class ValueType>
        static DeviceImage From(View<Device> device, ImageUsage usage, Format format, uint32_t width, uint32_t height,
                                const CSpan<ValueType> data, uint32_t mipLevels = 1,
                                SampleCount sampleCount = SampleCount::Sample1, ImageType type = ImageType::T2D,
                                SharingMode sharingMode = SharingMode::Exclusive,
                                ImageTiling tiling = ImageTiling::Optimal, bool mappable = false);

        static DeviceImage From(View<Device> device, ImageUsage usage, Format format, uint32_t width, uint32_t height,
                                const CSpan<uint8_t> data, uint32_t mipLevels = 1,
                                SampleCount sampleCount = SampleCount::Sample1, ImageType type = ImageType::T2D,
                                SharingMode sharingMode = SharingMode::Exclusive,
                                ImageTiling tiling = ImageTiling::Optimal, bool mappable = false);

        DeviceImage() = default;

        DeviceImage(View<Device> device, Extent3D size, ImageUsage usage, Format format, uint32_t mipLevels = 1,
                    SampleCount sampleCount = SampleCount::Sample1, ImageType type = ImageType::T2D,
                    SharingMode sharingMode = SharingMode::Exclusive, ImageTiling tiling = ImageTiling::Optimal,
                    bool mappable = false);
        DeviceImage(View<Device> device, ImageBuilder builder);
        DeviceImage(View<Device> device, VkImage image, Extent3D size, ImageUsage usage, Format format,
                    SharingMode sharingMode = SharingMode::Exclusive, ImageTiling tiling = ImageTiling::Optimal,
                    bool mappable = false);

        DeviceImage(const DeviceImage&)            = delete;
        DeviceImage& operator=(const DeviceImage&) = delete;

        DeviceImage(DeviceImage&& other) noexcept;
        DeviceImage& operator=(DeviceImage&& other) noexcept;

        ~DeviceImage() override;

        template <class Type>
        Type* map();
        template <class Type>
        const Type*    map() const;
        uint8_t*       map();
        const uint8_t* map() const;
        void           unmap() const;

        SubresourceLayout getSubresourceLayout(const ImageAspect aspect, uint32_t mipLevel = 0,
                                               uint32_t arrayLayer = 0) const;

        inline Extent3D      getSize() const;
        inline ImageUsage    getUsage() const;
        inline Format        getFormat() const;
        inline uint32_t      getMipLevels() const;
        inline SampleCount   getSampleCount() const;
        inline ImageType     getImageType() const;
        inline SharingMode   getSharingMode() const;
        inline VmaAllocation getAllocation() const;

      private:
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        Extent3D    m_size;
        ImageUsage  m_usage;
        Format      m_format;
        uint32_t    m_mipLevels = 1;
        SampleCount m_sampleCount;
        ImageType   m_type = ImageType::T2D;
        SharingMode m_sharingMode;
        ImageTiling m_tiling;
        bool        m_mappable;
    };

    enum class ImageViewType
    {
        T1D       = VK_IMAGE_VIEW_TYPE_1D,
        T2D       = VK_IMAGE_VIEW_TYPE_2D,
        T3D       = VK_IMAGE_VIEW_TYPE_3D,
        Cube      = VK_IMAGE_VIEW_TYPE_CUBE,
        T1DArray  = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
        T2DArray  = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        CubeArray = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageViewType, VkImageViewType)

    class ImageView
    {
      public:
        ImageView() = default;

        ImageView(View<Device> device, View<DeviceImage> image, ImageViewType type, ImageAspect aspect, Format format,
                  uint32_t baseMipLevel, uint32_t levelCount);
        ImageView(View<Device> device, View<DeviceImage> image, ImageAspect aspect, ImageViewType type);
        ImageView(View<Device> device, View<DeviceImage> image, ImageAspect aspect);

        ImageView(const ImageView&)            = delete;
        ImageView& operator=(const ImageView&) = delete;

        ImageView(ImageView&&) noexcept;
        ImageView& operator=(ImageView&&) noexcept;

        ~ImageView();

        inline ImageAspect       getAspect() const;
        inline Format            getFormat() const;
        inline View<DeviceImage> getImage() const;
        inline VkImageView       getHandle() const;

      private:
        View<Device>      m_device;
        View<DeviceImage> m_image;
        VkImageView       m_handle = VK_NULL_HANDLE;

        ImageAspect m_aspect;
        Format      m_format;
    };

    struct SamplerBuilder
    {
        Filter      filter      = Filter::Linear;
        AddressMode addressMode = AddressMode::Repeat;
        MipmapMode  mipmapMode  = MipmapMode::Linear;
        BorderColor borderColor = BorderColor::IntOpaqueBlack;
    };

    class Sampler : public DeviceObject<VkSampler>
    {
      public:
        Sampler() = default;
        Sampler(View<Device> device, SamplerBuilder builder = {});

        Sampler(const Sampler&)            = delete;
        Sampler& operator=(const Sampler&) = delete;

        Sampler(Sampler&& other) noexcept;
        Sampler& operator=(Sampler&& other) noexcept;

        ~Sampler() override;

        inline Filter      getFilter() const;
        inline AddressMode getAddressMode() const;
        inline MipmapMode  getMipmapMode() const;
        inline BorderColor getBorderColor() const;

      private:
        Filter      m_filter;
        AddressMode m_addressMode;
        MipmapMode  m_mipmapMode;
        BorderColor m_borderColor;
    };

    class Texture
    {
      public:
        Texture(View<Device> device, View<DeviceImage> image, SamplerBuilder samplerSettings = {});

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

#include "vzt/vulkan/image.inl"

#endif // VZT_VULKAN_IMAGE_HPP
