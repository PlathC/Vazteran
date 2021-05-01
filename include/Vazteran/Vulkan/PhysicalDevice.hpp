#ifndef VAZTERAN_PHYSICALDEVICE_HPP
#define VAZTERAN_PHYSICALDEVICE_HPP

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

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class PhysicalDevice {
    public:
        PhysicalDevice(Instance* instance, VkSurfaceKHR surface,
                       const std::vector<const char*>& deviceExtensions = PhysicalDevice::DefaultDeviceExtensions);

        VkPhysicalDevice VkHandle() const { return m_vkHandle; }
        std::vector<const char*> Extensions() { return m_extensions; }
        SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface) const;
        QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface) const;

        ~PhysicalDevice();

    private:
        static VkPhysicalDevice FindBestDevice(Instance* instance, VkSurfaceKHR surface,
                                               const std::vector<const char*>& deviceExtensions);
        static const std::vector<const char*> DefaultDeviceExtensions;

        VkPhysicalDevice m_vkHandle;
        std::vector<const char*> m_extensions;
    };

    static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
}

#endif //VAZTERAN_PHYSICALDEVICE_HPP
