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
        PhysicalDevice(vzt::Instance* instance, VkSurfaceKHR surface,
                       const std::vector<const char*>& deviceExtensions = vzt::PhysicalDevice::DefaultDeviceExtensions);

        VkPhysicalDevice VkHandle() const { return m_vkHandle; }
        std::vector<const char*> Extensions() { return m_extensions; }
        vzt::SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface) const;
        vzt::QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface) const;
        VkFormat FindDepthFormat();
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                     VkFormatFeatureFlags features);
        VkSampleCountFlagBits MaxUsableSampleCount();

        ~PhysicalDevice();

    private:
        static VkPhysicalDevice FindBestDevice(vzt::Instance* instance, VkSurfaceKHR surface,
                                               const std::vector<const char*>& deviceExtensions);
        static const std::vector<const char*> DefaultDeviceExtensions;

        VkPhysicalDevice m_vkHandle;
        std::vector<const char*> m_extensions;
    };

    static bool HasStencilComponent(VkFormat format);

    static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
    VkSampleCountFlagBits MaxUsableSampleCount(VkPhysicalDevice physicalDevice);
}

#endif //VAZTERAN_PHYSICALDEVICE_HPP
