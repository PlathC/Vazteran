#ifndef VZT_VULKAN_IMAGE_HPP
#define VZT_VULKAN_IMAGE_HPP

#include <vk_mem_alloc.h>

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"
#include "vzt/Vulkan/Format.hpp"

namespace vzt
{
    class Device;

    enum class ImageAspect : uint32_t
    {
        Color           = VK_IMAGE_ASPECT_COLOR_BIT,
        Depth           = VK_IMAGE_ASPECT_DEPTH_BIT,
        Stencil         = VK_IMAGE_ASPECT_STENCIL_BIT,
        MetaData        = VK_IMAGE_ASPECT_METADATA_BIT,
        Plane0          = VK_IMAGE_ASPECT_PLANE_0_BIT,
        Plane1          = VK_IMAGE_ASPECT_PLANE_1_BIT,
        Plane2          = VK_IMAGE_ASPECT_PLANE_2_BIT,
        MemoryPlane0Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
        MemoryPlane1Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
        MemoryPlane2Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
        MemoryPlane3Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
        Plane0KHR       = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR,
        Plane1KHR       = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR,
        Plane2KHR       = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageAspect, VkImageAspectFlags)
    VZT_DEFINE_BITWISE_FUNCTIONS(ImageAspect)

    enum class ImageLayout : uint32_t
    {
        Undefined                                = VK_IMAGE_LAYOUT_UNDEFINED,
        General                                  = VK_IMAGE_LAYOUT_GENERAL,
        ColorAttachmentOptimal                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        DepthStencilAttachmentOptimal            = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        DepthStencilReadOnlyOptimal              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        ShaderReadOnlyOptimal                    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        TransferSrcOptimal                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        TransferDstOptimal                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        Preinitialized                           = VK_IMAGE_LAYOUT_PREINITIALIZED,
        DepthReadOnlyStencilAttachmentOptimal    = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        DepthAttachmentStencilReadOnlyOptimal    = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        DepthAttachmentOptimal                   = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        DepthReadOnlyOptimal                     = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        StencilAttachmentOptimal                 = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        StencilReadOnlyOptimal                   = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        PresentSrcKHR                            = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        SharedPresentKHR                         = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
        FragmentDensityMapOptimalExt             = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
        FragmentShadingRateAttachmentOptimalKHR  = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
        ReadOnlyOptimal                          = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        AttachmentOptimal                        = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        DepthReadOnlyStencilAttachmentOptimalKHR = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        DepthAttachmentStencilReadOnlyOptimalKHR = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        ShadingRateOptimal                       = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
        DepthAttachmentOptimalKHR                = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        DepthReadOnlyOptimalKHR                  = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        StencilAttachmentOptimalKHR              = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        StencilReadOnlyOptimalKHR                = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageLayout, VkImageLayout)

    enum class ImageUsage : uint32_t
    {
        TransferSrc                   = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        TransferDst                   = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        Sampled                       = VK_IMAGE_USAGE_SAMPLED_BIT,
        Storage                       = VK_IMAGE_USAGE_STORAGE_BIT,
        ColorAttachment               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        DepthStencilAttachment        = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        TransientAttachment           = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        InputAttachment               = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        FragmentDensityMap            = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
        FragmentShadingRateAttachment = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
        InvocationMask                = VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI,
        ShadingRateImage              = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageUsage, VkImageUsageFlags)
    VZT_DEFINE_BITWISE_FUNCTIONS(ImageUsage)

    enum class ImageType
    {
        T1D = VK_IMAGE_TYPE_1D,
        T2D = VK_IMAGE_TYPE_2D,
        T3D = VK_IMAGE_TYPE_3D
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageType, VkImageType)

    enum class SampleCount : uint32_t
    {
        Sample1  = VK_SAMPLE_COUNT_1_BIT,
        Sample2  = VK_SAMPLE_COUNT_2_BIT,
        Sample4  = VK_SAMPLE_COUNT_4_BIT,
        Sample8  = VK_SAMPLE_COUNT_8_BIT,
        Sample16 = VK_SAMPLE_COUNT_16_BIT,
        Sample32 = VK_SAMPLE_COUNT_32_BIT,
        Sample64 = VK_SAMPLE_COUNT_64_BIT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(SampleCount, VkSampleCountFlagBits)

    enum class SharingMode
    {
        Exclusive  = VK_SHARING_MODE_EXCLUSIVE,
        Concurrent = VK_SHARING_MODE_CONCURRENT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(SharingMode, VkSharingMode)

    struct ImageBuilder
    {
        Extent3D    size;
        ImageUsage  usage;
        Format      format;
        uint32_t    mipLevels   = 1;
        ImageLayout layout      = ImageLayout::Undefined;
        SampleCount sampleCount = SampleCount::Sample1;
        ImageType   type        = ImageType::T2D;
        SharingMode sharingMode = SharingMode::Concurrent;
    };

    class Image
    {
      public:
        Image(View<Device> device, Extent3D size, ImageUsage usage, Format format, uint32_t mipLevels = 1,
              ImageLayout layout = ImageLayout::Undefined, SampleCount sampleCount = SampleCount::Sample1,
              ImageType type = ImageType::T2D, SharingMode sharingMode = SharingMode::Exclusive);
        Image(View<Device> device, ImageBuilder builder);
        Image(View<Device> device, VkImage image, Extent3D size, ImageUsage usage, Format format,
              SharingMode sharingMode = SharingMode::Exclusive);

        Image(const Image&)            = delete;
        Image& operator=(const Image&) = delete;

        Image(Image&&) noexcept;
        Image& operator=(Image&&) noexcept;

        ~Image();

        inline Extent3D    getSize() const;
        inline ImageUsage  getUsage() const;
        inline Format      getFormat() const;
        inline uint32_t    getMipLevels() const;
        inline ImageLayout getLayout() const;
        inline SampleCount getSampleCount() const;
        inline ImageType   getImageType() const;
        inline SharingMode getSharingMode() const;
        inline VkImage     getHandle() const;

      private:
        View<Device>  m_device     = {};
        VkImage       m_handle     = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        Extent3D    m_size;
        ImageUsage  m_usage;
        Format      m_format;
        uint32_t    m_mipLevels = 1;
        ImageLayout m_layout;
        SampleCount m_sampleCount;
        ImageType   m_type = ImageType::T2D;
        SharingMode m_sharingMode;
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
        ImageView(View<Device> device, View<Image> image, ImageViewType type, ImageAspect aspect, Format format,
                  uint32_t baseMipLevel, uint32_t levelCount);
        ImageView(View<Device> device, View<Image> image, ImageAspect aspect, ImageViewType type);
        ImageView(View<Device> device, View<Image> image, ImageAspect aspect);

        ImageView(const ImageView&)            = delete;
        ImageView& operator=(const ImageView&) = delete;

        ImageView(ImageView&&) noexcept;
        ImageView& operator=(ImageView&&) noexcept;

        ~ImageView();

        inline ImageAspect getAspect() const;
        inline Format      getFormat() const;
        inline View<Image> getImage() const;
        inline VkImageView getHandle() const;

      private:
        View<Device> m_device;
        View<Image>  m_image;
        VkImageView  m_handle = VK_NULL_HANDLE;

        ImageAspect m_aspect;
        Format      m_format;
    };
} // namespace vzt

#include "vzt/Vulkan/Image.inl"

#endif // VZT_VULKAN_IMAGE_HPP
