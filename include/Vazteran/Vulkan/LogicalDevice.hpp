
#ifndef VAZTERAN_LOGICALDEVICE_HPP
#define VAZTERAN_LOGICALDEVICE_HPP

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Vazteran/Vulkan/PhysicalDevice.hpp"

namespace vzt {
    using SingleTimeCommandFunction = std::function<void(VkCommandBuffer)>;

    class Instance;

    class LogicalDevice {
    public:
        LogicalDevice(vzt::Instance* instance, vzt::PhysicalDevice* parent, VkSurfaceKHR surface);

        LogicalDevice(const LogicalDevice&) = delete;
        LogicalDevice& operator=(const LogicalDevice&) = delete;

        LogicalDevice(LogicalDevice&&) noexcept;
        LogicalDevice& operator=(LogicalDevice&&) noexcept;

        void CreateBuffer(VkBuffer& buffer, VmaAllocation& bufferAllocation, VkDeviceSize size,
            VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags preferredFlags = 0);
        void CreateImage(VkImage& image, VmaAllocation& allocation, uint32_t width, uint32_t height,
            VkFormat format, VkSampleCountFlagBits numSamples, VkImageTiling tiling, VkImageUsageFlags usage);
        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                   VkImageLayout newLayout, VkImageAspectFlags aspectFlags);
        void SingleTimeCommand(const SingleTimeCommandFunction& singleTimeCommandFunction);
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        VmaAllocator AllocatorHandle() const { return m_allocator; }
        VkDevice VkHandle() const { return m_vkHandle; }
        VkQueue GraphicsQueue() const { return m_graphicsQueue; }
        VkQueue PresentQueue() const { return m_presentQueue; }
        vzt::QueueFamilyIndices DeviceQueueFamilyIndices() const { return m_queueFamilyIndices; };
        vzt::PhysicalDevice* Parent() const { return m_parent; }

        ~LogicalDevice();

    private:
        VmaAllocator m_allocator;
        vzt::PhysicalDevice* m_parent;
        VkDevice m_vkHandle{};
        VkPhysicalDeviceFeatures m_deviceFeatures{};
        VkQueue m_graphicsQueue{};
        VkQueue m_presentQueue{};

        vzt::QueueFamilyIndices m_queueFamilyIndices;
    };
}

#endif //VAZTERAN_LOGICALDEVICE_HPP
