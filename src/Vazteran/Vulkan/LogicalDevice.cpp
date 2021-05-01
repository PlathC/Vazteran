#include <set>
#include <stdexcept>
#include <vector>

#include "Vazteran/Vulkan/Instance.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/PhysicalDevice.hpp"

namespace vzt {
    LogicalDevice::LogicalDevice(Instance* instance, PhysicalDevice* parent, VkSurfaceKHR surface):
            m_parent(parent) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        auto deviceQueueFamilyIndices = parent->FindQueueFamilies(surface);
        std::set<uint32_t> uniqueQueueFamilies = {
                deviceQueueFamilyIndices.graphicsFamily.value(),
                deviceQueueFamilyIndices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfo.pNext = nullptr;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &m_deviceFeatures;
        auto deviceExtensions = parent->Extensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        auto validationLayers = instance->ValidationLayers();
        if (Instance::EnableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_parent->VkHandle(), &createInfo, nullptr, &m_handle) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        vkGetDeviceQueue(m_handle, deviceQueueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_handle, deviceQueueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);
    }

    LogicalDevice::~LogicalDevice() {
        vkDestroyDevice(m_handle, nullptr);
    }
}
