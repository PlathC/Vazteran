//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#include "Vazteran/Render/Vulkan/DeviceManager.hpp"

namespace vzt
{
    DeviceManager::DeviceManager(std::shared_ptr<VkInstance> instance, Surface* surface,
            const std::vector<const char*>& neededDeviceExtensions):
            m_vkInstance(std::move(instance))
    {
        if(!neededDeviceExtensions.empty())
        {
            m_deviceExtensions = neededDeviceExtensions;
        }
        else
        {
            m_deviceExtensions = std::vector<const char*>(DefautlDeviceExtensions.begin(), DefautlDeviceExtensions.end());
        }

        vkEnumeratePhysicalDevices(*m_vkInstance, &m_deviceCount, nullptr);
        if (m_deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        auto rawDevices = std::vector<VkPhysicalDevice>(m_deviceCount);
        vkEnumeratePhysicalDevices(*m_vkInstance, &m_deviceCount, rawDevices.data());
        auto devices = std::vector<PhysicalDevice>(m_deviceCount);
        for(std::size_t i = 0; i < devices.size(); i++)
        {
            devices[i] = PhysicalDevice(rawDevices[i]);
        }

        try
        {
            m_physicalDevice = std::make_shared<PhysicalDevice>(PickBestSuitableDevice(devices, surface));
        }
        catch(const std::exception& e)
        {
            throw e;
        }

        QueueFamilyIndices indices = m_physicalDevice->FindQueueFamilies(surface);
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        m_physicalDevice->InitializeDevice(uniqueQueueFamilies, m_deviceExtensions);

        m_physicalDevice->CreateQueue(indices.graphicsFamily.value(), m_graphicsQueue);
        m_physicalDevice->CreateQueue(indices.presentFamily.value(), m_presentQueue);
    }

    std::shared_ptr<PhysicalDevice> DeviceManager::PickedPhysicalDevice()
    {
        return m_physicalDevice;
    }

    PhysicalDevice DeviceManager::PickBestSuitableDevice(const std::vector<PhysicalDevice>& devices, Surface* surface)
    {
        std::multimap<int, PhysicalDevice> candidates;

        for (const auto& device : devices)
        {
            int score = device.Rate(surface, m_deviceExtensions);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0)
        {
            return candidates.rbegin()->second;
        }
        else
        {
            // TODO: Add proper logger
            throw std::runtime_error("[ERROR] Failed to find a suitable GPU!");
        }
    }
}