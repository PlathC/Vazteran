#include "vzt/vulkan/image.hpp"

#include <utility>

#include "vzt/core/logger.hpp"
#include "vzt/vulkan/buffer.hpp"
#include "vzt/vulkan/command.hpp"
#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/image.hpp"

namespace vzt
{
    DeviceImage DeviceImage::From(View<Device> device, ImageUsage usage, Format format, uint32_t width, uint32_t height,
                                  const CSpan<uint8_t> data, uint32_t mipLevels, SampleCount sampleCount,
                                  ImageType type, SharingMode sharingMode, ImageTiling tiling, bool)
    {
        DeviceImage deviceImage{
            device,
            Extent2D{width, height},
            usage | vzt::ImageUsage::TransferDst,
            format,
            mipLevels,
            sampleCount,
            type,
            sharingMode,
            tiling,
        };
        auto staging = Buffer::From(device, data, vzt::BufferUsage::TransferSrc, vzt::MemoryLocation::Device);

        const auto graphicsQueue = device->getQueue(vzt::QueueType::Graphics);
        graphicsQueue->oneShot([&](vzt::CommandBuffer& commands) {
            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = deviceImage;
            imageBarrier.dst       = Access::TransferWrite;
            imageBarrier.oldLayout = vzt::ImageLayout::Undefined;
            imageBarrier.newLayout = vzt::ImageLayout::TransferDstOptimal;
            commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::Transfer, imageBarrier);

            commands.copy(staging, deviceImage, width, height);
        });

        return deviceImage;
    }

    DeviceImage::DeviceImage(View<Device> device, Extent3D size, ImageUsage usage, Format format, uint32_t mipLevels,
                             SampleCount sampleCount, ImageType type, SharingMode sharingMode, ImageTiling tiling,
                             bool mappable)
        : DeviceObject<VkImage>(device), m_size(size), m_usage(usage), m_format(format), m_mipLevels(mipLevels),
          m_sampleCount(sampleCount), m_type(type), m_sharingMode(sharingMode), m_tiling(tiling), m_mappable(mappable)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = toVulkan(m_type);
        imageInfo.format        = toVulkan(m_format);
        imageInfo.extent        = {m_size.width, m_size.height, m_size.depth};
        imageInfo.mipLevels     = m_mipLevels;
        imageInfo.arrayLayers   = 1;
        imageInfo.samples       = toVulkan(m_sampleCount);
        imageInfo.tiling        = toVulkan(m_tiling);
        imageInfo.usage         = toVulkan(m_usage | ImageUsage::Sampled);
        imageInfo.sharingMode   = toVulkan(m_sharingMode);
        imageInfo.initialLayout = toVulkan(ImageLayout::Undefined);

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
        if (m_mappable)
            allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        vkCheck(vmaCreateImage(m_device->getAllocator(), &imageInfo, &allocInfo, &m_handle, &m_allocation, nullptr),
                "Can't allocate image.");
    }

    DeviceImage::DeviceImage(View<Device> device, ImageBuilder builder)
        : DeviceImage(device, builder.size, builder.usage, builder.format, builder.mipLevels, builder.sampleCount,
                      builder.type, builder.sharingMode, builder.tiling, builder.mappable)
    {
    }

    DeviceImage::DeviceImage(View<Device> device, VkImage image, Extent3D size, ImageUsage usage, Format format,
                             SharingMode sharingMode, ImageTiling tiling, bool mappable)
        : DeviceObject(device, image), m_size(size), m_usage(usage), m_format(format), m_sharingMode(sharingMode),
          m_tiling(tiling), m_mappable(mappable)
    {
    }

    DeviceImage::DeviceImage(DeviceImage&& other) noexcept
        : DeviceObject(std::move(other)), m_allocation(std::exchange(other.m_allocation, VK_NULL_HANDLE)),
          m_size(std::move(other.m_size)), m_usage(std::move(other.m_usage)), m_format(std::move(other.m_format)),
          m_mipLevels(std::move(other.m_mipLevels)), m_sampleCount(std::move(other.m_sampleCount)),
          m_type(std::move(other.m_type)), m_sharingMode(std::move(other.m_sharingMode)),
          m_tiling(std::move(other.m_tiling)), m_mappable(other.m_mappable)
    {
    }

    DeviceImage& DeviceImage::operator=(DeviceImage&& other) noexcept
    {
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_size, other.m_size);
        std::swap(m_usage, other.m_usage);
        std::swap(m_format, other.m_format);
        std::swap(m_mipLevels, other.m_mipLevels);
        std::swap(m_sampleCount, other.m_sampleCount);
        std::swap(m_type, other.m_type);
        std::swap(m_sharingMode, other.m_sharingMode);
        std::swap(m_tiling, other.m_tiling);
        std::swap(m_mappable, other.m_mappable);

        DeviceObject<VkImage>::operator=(std::move(other));
        return *this;
    }

    DeviceImage::~DeviceImage()
    {
        // If the object did not created the handle (i.e. for swapchain images)
        if (m_allocation == VK_NULL_HANDLE)
            return;

        vmaDestroyImage(m_device->getAllocator(), m_handle, m_allocation);
    }

    uint8_t* DeviceImage::map()
    {
        assert(m_mappable && "Device image has not been tagged as mappable at creation!");

        uint8_t* mappedData;
        vmaMapMemory(m_device->getAllocator(), m_allocation, reinterpret_cast<void**>(&mappedData));

        return mappedData;
    }

    const uint8_t* DeviceImage::map() const
    {
        assert(m_mappable && "Device image has not been tagged as mappable at creation!");

        uint8_t* mappedData;
        vmaMapMemory(m_device->getAllocator(), m_allocation, reinterpret_cast<void**>(&mappedData));

        return mappedData;
    }

    void DeviceImage::unmap() const { vmaUnmapMemory(m_device->getAllocator(), m_allocation); }

    SubresourceLayout DeviceImage::getSubresourceLayout(const ImageAspect, uint32_t, uint32_t) const
    {
        VkImageSubresource  subResource{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
        VkSubresourceLayout subResourceLayout;
        vkGetImageSubresourceLayout(m_device->getHandle(), m_handle, &subResource, &subResourceLayout);

        return {subResourceLayout.offset, subResourceLayout.size, subResourceLayout.rowPitch,
                subResourceLayout.arrayPitch, subResourceLayout.depthPitch};
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

    Sampler::Sampler(View<Device> device, SamplerBuilder builder)
        : DeviceObject<VkSampler>(device), m_filter(builder.filter), m_addressMode(builder.addressMode),
          m_mipmapMode(builder.mipmapMode), m_borderColor(builder.borderColor)
    {
        const PhysicalDevice             hardware   = m_device->getHardware();
        const VkPhysicalDeviceProperties properties = hardware.getProperties();

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = toVulkan(m_filter);
        samplerInfo.minFilter               = toVulkan(m_filter);
        samplerInfo.addressModeU            = toVulkan(m_addressMode);
        samplerInfo.addressModeV            = toVulkan(m_addressMode);
        samplerInfo.addressModeW            = toVulkan(m_addressMode);
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = toVulkan(m_borderColor);
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = toVulkan(m_mipmapMode);

        vkCheck(vkCreateSampler(m_device->getHandle(), &samplerInfo, nullptr, &m_handle),
                "Failed to create texture sampler!");
    }

    Sampler::Sampler(Sampler&& other) noexcept
        : DeviceObject<VkSampler>(std::move(other)), m_filter(std::move(other.m_filter)),
          m_addressMode(std::move(other.m_addressMode)), m_mipmapMode(std::move(other.m_mipmapMode)),
          m_borderColor(std::move(other.m_borderColor))
    {
    }

    Sampler& Sampler::operator=(Sampler&& other) noexcept
    {
        std::swap(m_filter, other.m_filter);
        std::swap(m_addressMode, other.m_addressMode);
        std::swap(m_mipmapMode, other.m_mipmapMode);
        std::swap(m_borderColor, other.m_borderColor);

        DeviceObject<VkSampler>::operator=(std::move(other));
        return *this;
    }

    Sampler::~Sampler()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        vkDestroySampler(m_device->getHandle(), m_handle, nullptr);
    }

    Texture::Texture(View<Device> device, View<DeviceImage> image, SamplerBuilder samplerSettings)
        : m_sampler(device, samplerSettings),
          m_imageView(device, image,
                      any(image->getUsage() & ImageUsage::DepthStencilAttachment) ? ImageAspect::Depth
                                                                                  : ImageAspect::Color)
    {
    }

    Texture::Texture(Texture&& other) noexcept
        : m_sampler(std::move(other.m_sampler)), m_imageView(std::move(other.m_imageView))
    {
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        std::swap(m_sampler, other.m_sampler);
        std::swap(m_imageView, other.m_imageView);

        return *this;
    }
} // namespace vzt
