#include "vzt/Device.hpp"

#include <set>
#include <stdexcept>

#define VMA_IMPLEMENTATION

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "vzt/Instance.hpp"

namespace vzt
{
    bool hasSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        std::vector<VkSurfaceFormatKHR> formats;
        if (formatCount > 0)
        {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        std::vector<VkPresentModeKHR> presentModes;
        if (presentModeCount > 0)
        {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
        }

        return !formats.empty() && !presentModes.empty();
    }

    PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) : m_handle(handle) {}

    bool PhysicalDevice::isSuitable(DeviceConfiguration configuration, View<Surface> surface) const
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies{queueFamilyCount};
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, queueFamilies.data());

        VkBool32 presentSupport = false;
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            const auto& queueFamily = queueFamilies[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                configuration.queueTypes = configuration.queueTypes & ~(QueueType::Graphics);

            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                configuration.queueTypes = configuration.queueTypes & ~(QueueType::Compute);

            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                configuration.queueTypes = configuration.queueTypes & ~(QueueType::Transfer);

            if (!surface || presentSupport)
                continue;

            vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, i, surface->getHandle(), &presentSupport);
        }

        bool isSuitable = !any(configuration.queueTypes) && hasExtensions(configuration.extensions);
        if (surface)
            isSuitable &= hasSwapchain(m_handle, surface->getHandle());

        if (configuration.hasAnisotropy)
        {
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(m_handle, &supportedFeatures);
            isSuitable &= static_cast<bool>(supportedFeatures.samplerAnisotropy);
        }

        return isSuitable;
    }

    bool PhysicalDevice::hasExtensions(const std::vector<const char*>& deviceExtensions) const
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions{deviceExtensions.begin(), deviceExtensions.end()};
        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    Device::Device(View<Instance> instance, PhysicalDevice device, DeviceConfiguration configuration)
        : m_instance(instance), m_device(device)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        QueueType   queueTypes    = configuration.queueTypes;
        const float queuePriority = 1.0f;

        std::unordered_map<QueueType, uint32_t> queueIds{};
        uint32_t                                queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_device.getHandle(), &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies{queueFamilyCount};
        vkGetPhysicalDeviceQueueFamilyProperties(m_device.getHandle(), &queueFamilyCount, queueFamilies.data());
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            const auto& queueFamily = queueFamilies[i];

            QueueType selected = QueueType::None;
            if (any(queueTypes & QueueType::Graphics) && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                selected = QueueType::Graphics;
            else if (any(queueTypes & QueueType::Compute) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
                selected = QueueType::Compute;
            else if (any(queueTypes & QueueType::Transfer) && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT))
                selected = QueueType::Transfer;

            queueIds[selected] = i;

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = i;
            queueCreateInfo.queueCount       = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfo.pNext            = nullptr;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(m_device.getHandle(), &supportedFeatures);

        VkDeviceCreateInfo createInfo{};
        createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos       = queueCreateInfos.data();
        createInfo.pEnabledFeatures        = &supportedFeatures;
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(configuration.extensions.size());
        createInfo.ppEnabledExtensionNames = configuration.extensions.data();

        const auto& instanceConfiguration = instance->getConfiguration();
        createInfo.enabledLayerCount      = 0;
        if (instanceConfiguration.enableValidation)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(instanceConfiguration.validationLayers.size());
            createInfo.ppEnabledLayerNames = instanceConfiguration.validationLayers.data();
        }

        vkCheck(vkCreateDevice(m_device.getHandle(), &createInfo, nullptr, &m_handle),
                "Failed to create logical device.");

        queueTypes = configuration.queueTypes;
        for (auto [type, id] : queueIds)
            m_queues.emplace(this, type, id);

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo                        = {};

        allocatorInfo.physicalDevice   = m_device.getHandle();
        allocatorInfo.device           = m_handle;
        allocatorInfo.instance         = instance->getHandle();
        allocatorInfo.vulkanApiVersion = getAPIVersion();
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
        vkCheck(vmaCreateAllocator(&allocatorInfo, &m_allocator), "Failed to create allocator.");
    }

    Device::Device(Device&& other) noexcept : m_device(std::move(other.m_device))
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_handle, other.m_handle);
        std::swap(m_allocator, other.m_allocator);
    }

    Device& Device::operator=(Device&& other) noexcept
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_allocator, other.m_allocator);

        return *this;
    }

    Device::~Device()
    {
        if (m_allocator != VK_NULL_HANDLE)
            vmaDestroyAllocator(m_allocator);

        if (m_handle != VK_NULL_HANDLE)
            vkDestroyDevice(m_handle, nullptr);
    }

    Queue::Queue(View<Device> device, QueueType type, uint32_t id) : m_device(device), m_type(type), m_id(id)
    {
        vkGetDeviceQueue(device->getHandle(), id, 0, &m_handle);
    }

} // namespace vzt
