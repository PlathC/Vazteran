#include <set>
#include <string>
#include <stdexcept>

#include "Vazteran/Vulkan/Instance.hpp"
#include "Vazteran/Vulkan/PhysicalDevice.hpp"

namespace vzt {
    const std::vector<const char*> PhysicalDevice::DefaultDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkPhysicalDevice PhysicalDevice::FindBestDevice(vzt::Instance* instance, VkSurfaceKHR surface,
                                                    const std::vector<const char*>& deviceExtensions) {
        std::vector<VkPhysicalDevice> physicalDevices = instance->EnumeratePhysicalDevice();
        for (const auto& device : physicalDevices) {
            if (IsDeviceSuitable(device, surface, deviceExtensions)) {
                return device;
            }
        }
        return VK_NULL_HANDLE;
    }

    PhysicalDevice::PhysicalDevice(vzt::Instance* instance, VkSurfaceKHR surface,
                                   const std::vector<const char*>& deviceExtensions) :
            m_vkHandle(FindBestDevice(instance, surface, deviceExtensions)), m_extensions(deviceExtensions) {
        if (m_vkHandle == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    SwapChainSupportDetails PhysicalDevice::QuerySwapChainSupport(VkSurfaceKHR surface) const {
        return vzt::QuerySwapChainSupport(m_vkHandle, surface);
    }

    QueueFamilyIndices PhysicalDevice::FindQueueFamilies(VkSurfaceKHR surface) const {
        return vzt::FindQueueFamilies(m_vkHandle, surface);
    }

    VkFormat PhysicalDevice::FindDepthFormat() {
        return FindSupportedFormat(
                { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                                 VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_vkHandle, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    static bool HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    PhysicalDevice::~PhysicalDevice() { }

    static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
                                 const std::vector<const char*>& deviceExtensions) {
        QueueFamilyIndices indices = vzt::FindQueueFamilies(device, surface);

        bool extensionsSupported = vzt::CheckDeviceExtensionSupport(device, deviceExtensions);
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            vzt::SwapChainSupportDetails swapChainSupport = vzt::QuerySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    static vzt::SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        vzt::SwapChainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        vzt::QueueFamilyIndices indices{};

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        auto queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.IsComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

}
