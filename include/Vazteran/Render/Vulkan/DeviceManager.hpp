//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_DEVICEMANAGER_HPP
#define VAZTERAN_DEVICEMANAGER_HPP

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

#include "Vazteran/Render/Vulkan/Surface.hpp"

namespace vzt
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, Surface* surface)
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // TODO: Add dynamic way to manage queues features
        for (uint32_t i = 0; i < queueFamilies.size(); i++)
        {
            if (surface->CheckSupport(device, i))
            {
                indices.presentFamily = i;
            }

            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            if(indices.IsComplete())
                break;
        }

        return indices;
    }

    static int RateDeviceSuitability(VkPhysicalDevice device, Surface* surface)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // TODO: Implement dynamic way to manage device score
        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 1000;
        }

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader)
        {
            return 0;
        }

        if(!FindQueueFamilies(device, surface).IsComplete())
        {
            return 0;
        }

        return score;
    }

    static VkPhysicalDevice PickBestSuitableDevice(const std::vector<VkPhysicalDevice>& devices, Surface* surface)
    {
        std::multimap<int, VkPhysicalDevice> candidates;
        VkPhysicalDevice result;

        for (const auto& device : devices)
        {
            int score = RateDeviceSuitability(device, surface);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0)
        {
            result = candidates.rbegin()->second;
        }
        else
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        return result;
    }

    using TestSuitableDevice = std::function<bool(VkPhysicalDevice)>;

    class DeviceManager
    {
    public:
        explicit DeviceManager(std::shared_ptr<VkInstance> instance, Surface* surface);
        ~DeviceManager();
    private:
        TestSuitableDevice m_suitableDeviceTest;

        uint32_t m_deviceCount = 0;
        std::shared_ptr<VkInstance> m_vkInstance;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device;

        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
    };
}


#endif //VAZTERAN_DEVICEMANAGER_HPP
