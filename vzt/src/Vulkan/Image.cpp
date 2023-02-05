#include "vzt/Vulkan/Image.hpp"

#include "vzt/Core/Logger.hpp"
#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    DeviceImage::DeviceImage(View<Device> device, Extent3D size, ImageUsage usage, Format format, uint32_t mipLevels,
                             ImageLayout layout, SampleCount sampleCount, ImageType type, SharingMode sharingMode)
        : m_device(device), m_size(size), m_usage(usage), m_format(format), m_mipLevels(mipLevels), m_layout(layout),
          m_sampleCount(sampleCount), m_type(type), m_sharingMode(sharingMode)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = toVulkan(m_type);
        imageInfo.format        = toVulkan(m_format);
        imageInfo.extent        = {m_size.width, m_size.height, m_size.depth};
        imageInfo.mipLevels     = m_mipLevels;
        imageInfo.arrayLayers   = 1;
        imageInfo.samples       = toVulkan(m_sampleCount);
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage         = toVulkan(m_usage | vzt::ImageUsage::Sampled);
        imageInfo.sharingMode   = toVulkan(m_sharingMode);
        imageInfo.initialLayout = toVulkan(m_layout);

        const auto         hardware = m_device->getHardware();
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(hardware.getHandle(), toVulkan(format), &properties);

        VmaAllocationCreateInfo imageAllocCreateInfo = {};
        imageAllocCreateInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

        vkCheck(vmaCreateImage(m_device->getAllocator(), &imageInfo, &imageAllocCreateInfo, &m_handle, &m_allocation,
                               nullptr),
                "Can't allocate image.");
    }

    DeviceImage::DeviceImage(View<Device> device, ImageBuilder builder)
        : DeviceImage(device, builder.size, builder.usage, builder.format, builder.mipLevels, builder.layout,
                      builder.sampleCount, builder.type, builder.sharingMode)
    {
    }

    DeviceImage::DeviceImage(View<Device> device, VkImage image, Extent3D size, ImageUsage usage, Format format,
                             SharingMode sharingMode)
        : m_device(device), m_handle(image), m_size(size), m_usage(usage), m_format(format), m_sharingMode(sharingMode)
    {
    }

    DeviceImage::DeviceImage(DeviceImage&& other) noexcept
        : m_device(other.m_device), m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE)),
          m_allocation(std::exchange(other.m_allocation, VK_NULL_HANDLE)), m_size(std::move(other.m_size)),
          m_usage(std::move(other.m_usage)), m_format(std::move(other.m_format)),
          m_mipLevels(std::move(other.m_mipLevels)), m_layout(std::move(other.m_layout)),
          m_sampleCount(std::move(other.m_sampleCount)), m_type(std::move(other.m_type)),
          m_sharingMode(std::move(other.m_sharingMode))
    {
    }

    DeviceImage& DeviceImage::operator=(DeviceImage&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_size, other.m_size);
        std::swap(m_usage, other.m_usage);
        std::swap(m_format, other.m_format);
        std::swap(m_mipLevels, other.m_mipLevels);
        std::swap(m_layout, other.m_layout);
        std::swap(m_sampleCount, other.m_sampleCount);
        std::swap(m_type, other.m_type);
        std::swap(m_sharingMode, other.m_sharingMode);

        return *this;
    }

    DeviceImage::~DeviceImage()
    {
        // If the object did not created the handle (i.e. for swapchain images)
        if (m_allocation == VK_NULL_HANDLE)
            return;

        vmaDestroyImage(m_device->getAllocator(), m_handle, m_allocation);
    }

    ImageView::ImageView(View<Device> device, View<DeviceImage> image, ImageViewType type, ImageAspect aspect,
                         Format format, uint32_t baseMipLevel, uint32_t levelCount)
        : m_device(device), m_image(image), m_aspect(aspect), m_format(format)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = m_image->getHandle();
        viewInfo.viewType                        = toVulkan(type);
        viewInfo.format                          = toVulkan(format);
        viewInfo.subresourceRange.aspectMask     = toVulkan(m_aspect);
        viewInfo.subresourceRange.baseMipLevel   = baseMipLevel;
        viewInfo.subresourceRange.levelCount     = levelCount;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        vkCheck(vkCreateImageView(m_device->getHandle(), &viewInfo, nullptr, &m_handle),
                "failed to create texture image view!");
    }

    ImageViewType toImageViewType(ImageType type)
    {
        if (type == ImageType::T1D)
            return ImageViewType::T1D;
        if (type == ImageType::T2D)
            return ImageViewType::T2D;
        if (type == ImageType::T3D)
            return ImageViewType::T3D;

        vzt::logger::error("Unknown conversion from ImageType to ImageViewType.");
        return ImageViewType::T2D;
    }

    ImageView::ImageView(View<Device> device, View<DeviceImage> image, ImageAspect aspect, ImageViewType type)
        : ImageView(device, image, type, aspect, image->getFormat(), 0, image->getMipLevels())
    {
    }

    ImageView::ImageView(View<Device> device, View<DeviceImage> image, ImageAspect aspect)
        : ImageView(device, image, toImageViewType(image->getImageType()), aspect, image->getFormat(), 0,
                    image->getMipLevels())
    {
    }

    ImageView::ImageView(ImageView&& other) noexcept
        : m_device(std::move(other.m_device)), m_image(std::move(other.m_image)),
          m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE)), m_aspect(std::move(other.m_aspect)),
          m_format(std::move(other.m_format))
    {
    }

    ImageView& ImageView::operator=(ImageView&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_image, other.m_image);
        std::swap(m_handle, other.m_handle);
        std::swap(m_aspect, other.m_aspect);
        std::swap(m_format, other.m_format);

        return *this;
    }

    ImageView::~ImageView()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        vkDestroyImageView(m_device->getHandle(), m_handle, nullptr);
    }

} // namespace vzt
