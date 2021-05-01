
#ifndef VAZTERAN_LOGICALDEVICE_HPP
#define VAZTERAN_LOGICALDEVICE_HPP

#include <vector>

#include <vulkan/vulkan.h>

namespace vzt {
    class Instance;
    class PhysicalDevice;

    class LogicalDevice {
    public:
        LogicalDevice(Instance* instance, PhysicalDevice* parent, VkSurfaceKHR surface);

        VkDevice VkHandle() { return m_handle; }
        VkQueue GraphicsQueue() const { return m_graphicsQueue; }
        VkQueue PresentQueue() const { return m_presentQueue; }
        PhysicalDevice* Parent() const { return m_parent; }

        ~LogicalDevice();

    private:
        PhysicalDevice* m_parent = nullptr;
        VkPhysicalDeviceFeatures m_deviceFeatures{};
        VkDevice m_handle{};
        VkQueue m_graphicsQueue{};
        VkQueue m_presentQueue{};
    };
}

#endif //VAZTERAN_LOGICALDEVICE_HPP
