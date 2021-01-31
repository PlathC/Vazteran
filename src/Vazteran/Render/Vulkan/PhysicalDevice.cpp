//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#include "Vazteran/Render/Vulkan/PhysicalDevice.hpp"

#include "Vazteran/Render/Vulkan/Surface.hpp"

namespace vzt
{
    PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& device):
            m_physicalDevice(device)
    {
        vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
        vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
    }

    int PhysicalDevice::Rate(Surface* surface, const std::vector<const char*>& extensions) const
    {
        // TODO: Implement dynamic way to manage device score
        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (m_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 1000;
        }

        // Application can't function without geometry shaders
        if (!m_features.geometryShader)
        {
            return 0;
        }

        bool extensionsSupported = CheckDeviceExtensionSupport(extensions);
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        if(!FindQueueFamilies(surface).IsComplete() || !extensionsSupported || !swapChainAdequate)
        {
            return 0;
        }

        return score;
    }

    void PhysicalDevice::InitializeDevice(const std::set<uint32_t>& uniqueQueueFamilies, const std::vector<const char*>& extensions)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = 1;

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        m_device = VkDevice{};
        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device.value()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }
    }

    void PhysicalDevice::CreateQueue(uint32_t id, VkQueue& queue) const
    {
        if(m_device)
            vkGetDeviceQueue(m_device.value(), id, 0, &queue);
        else
            throw std::runtime_error("This physical device does not contains any device.");
    }

    void PhysicalDevice::CreateSwapChain(VkSwapchainCreateInfoKHR& createInfo,
            VkSwapchainKHR& swapChain,
            std::vector<VkImage>& swapChainImages) const
    {
        if(m_device.has_value())
        {
            if (vkCreateSwapchainKHR(m_device.value(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create swap chain!");
            }

            uint32_t imageCount = 0;
            vkGetSwapchainImagesKHR(m_device.value(), swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(m_device.value(), swapChain, &imageCount, swapChainImages.data());
        }
        else
        {
            throw std::runtime_error("This physical device does not contains any device.");
        }
    }

    void PhysicalDevice::CreateShaderModule(const VkShaderModuleCreateInfo& createInfo, VkShaderModule& shaderModule)
    {
        if(m_device.has_value())
        {
            if (vkCreateShaderModule(m_device.value(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create shader module!");
            }
        }
        else
        {
            throw std::runtime_error("This physical device does not contains any device");
        }
    }

    void PhysicalDevice::CreatePipelineLayout(const VkPipelineLayoutCreateInfo &createInfo,
            VkPipelineLayout& pipelineLayout)
    {
        if(m_device.has_value())
        {
            if (vkCreatePipelineLayout(m_device.value(), &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create pipeline layout!");
            }
        }
        else
        {
            throw std::runtime_error("This physical device does not contains any device");
        }
    }

    void PhysicalDevice::CreateRenderPass(const VkRenderPassCreateInfo& createInfo, VkRenderPass& renderPass)
    {
        if(m_device.has_value())
        {
            if (vkCreateRenderPass(m_device.value(), &createInfo, nullptr, &renderPass) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create pipeline layout!");
            }
        }
        else
        {
            throw std::runtime_error("This physical device does not contains any device");
        }
    }

    const VkDevice& PhysicalDevice::DeviceHandler() const
    {
        if(m_device.has_value())
            return m_device.value();
        else
            throw std::runtime_error("This physical device does not contiains any device.");
    }

    QueueFamilyIndices PhysicalDevice::FindQueueFamilies(Surface* surface) const
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

        // TODO: Add dynamic way to manage queues features
        for (uint32_t i = 0; i < queueFamilies.size(); i++)
        {
            if (surface->CheckSupport(m_physicalDevice, i))
            {
                indices.presentFamily = i;
            }

            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            if (indices.IsComplete())
                break;
        }
        return indices;
    }

    SwapChainSupportDetails PhysicalDevice::QuerySwapChainSupport(Surface* surface) const
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, surface->SurfaceHandler(), &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface->SurfaceHandler(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface->SurfaceHandler(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface->SurfaceHandler(), &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface->SurfaceHandler(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    PhysicalDevice::~PhysicalDevice()
    {
        if(m_device)
            vkDestroyDevice(*m_device, nullptr);
    }

    bool PhysicalDevice::CheckDeviceExtensionSupport(const std::vector<const char*>& extensions) const
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
}