
#ifndef VAZTERAN_LOGICALDEVICE_HPP
#define VAZTERAN_LOGICALDEVICE_HPP

#include <vector>

#include <vulkan/vulkan.h>

#include <Vazteran/Vulkan/PhysicalDevice.hpp>

namespace vzt {
    class Instance;

    class LogicalDevice {
    public:
        LogicalDevice(Instance* instance, PhysicalDevice* parent, VkSurfaceKHR surface);

        void CreateBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
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
