
#ifndef VAZTERAN_LOGICALDEVICE_HPP
#define VAZTERAN_LOGICALDEVICE_HPP

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

#include <Vazteran/Vulkan/PhysicalDevice.hpp>

namespace vzt {
    using SingleTimeCommandFunction = std::function<void(VkCommandBuffer)>;

    class Instance;

    class LogicalDevice {
    public:
        LogicalDevice(Instance* instance, PhysicalDevice* parent, VkSurfaceKHR surface);

        void CreateBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties);
        void CreateImage(VkImage& image, VkDeviceMemory& imageMemory, uint32_t width, uint32_t height, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                   VkImageLayout newLayout, VkImageAspectFlags aspectFlags);
        void SingleTimeCommand(SingleTimeCommandFunction singleTimeCommandFunction);
        void AllocateMemory(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags properties);
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


        VkDevice VkHandle() { return m_vkHandle; }
        VkQueue GraphicsQueue() const { return m_graphicsQueue; }
        VkQueue PresentQueue() const { return m_presentQueue; }
        QueueFamilyIndices DeviceQueueFamilyIndices() const { return m_queueFamilyIndices; };
        PhysicalDevice* Parent() const { return m_parent; }

        ~LogicalDevice();

    private:
        PhysicalDevice* m_parent = nullptr;
        VkPhysicalDeviceFeatures m_deviceFeatures{};
        VkDevice m_vkHandle{};
        VkQueue m_graphicsQueue{};
        VkQueue m_presentQueue{};

        QueueFamilyIndices m_queueFamilyIndices;
    };
}

#endif //VAZTERAN_LOGICALDEVICE_HPP
