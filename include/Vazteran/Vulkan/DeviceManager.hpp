#ifndef VAZTERAN_DEVICEMANAGER_HPP
#define VAZTERAN_DEVICEMANAGER_HPP

#include <iostream>
#include <optional>
#include <vector>

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

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class DeviceManager {
    public:
        DeviceManager(Instance* instance, VkSurfaceKHR surface);

        VkPhysicalDevice PhysicalDevice() const { return m_physicalDevice; };
        VkDevice LogicalDevice() const { return m_device; };
        VkQueue GraphicsQueue() const { return m_graphicsQueue; }
        VkQueue PresentQueue() const { return m_presentQueue; }

        static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        ~DeviceManager();
    private:
        VkDevice m_device{};
        VkQueue m_graphicsQueue{};
        VkQueue m_presentQueue{};

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures m_deviceFeatures{};

        static const std::vector<const char*> DeviceExtensions;
    };

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
}

#endif //VAZTERAN_DEVICEMANAGER_HPP
