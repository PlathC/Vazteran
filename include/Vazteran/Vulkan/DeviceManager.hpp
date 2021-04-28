#ifndef VAZTERAN_DEVICEMANAGER_HPP
#define VAZTERAN_DEVICEMANAGER_HPP

#include <optional>

#include <vulkan/vulkan.h>

namespace vzt {
    class Instance;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();;
        }
    };

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    class DeviceManager {
    public:
        explicit DeviceManager(Instance* instance, VkSurfaceKHR surface);
        static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
        ~DeviceManager() { vkDestroyDevice(m_device, nullptr); }

    private:
        VkDevice m_device;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures m_deviceFeatures{};
    };
}

#endif //VAZTERAN_DEVICEMANAGER_HPP
