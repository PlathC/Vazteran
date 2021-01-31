//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_PHYSICALDEVICE_HPP
#define VAZTERAN_PHYSICALDEVICE_HPP

#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

namespace vzt
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const
        {

            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class Surface;

    class PhysicalDevice
    {
    public:
        PhysicalDevice() = default;
        explicit PhysicalDevice(const VkPhysicalDevice& device);

        int Rate(Surface* surface, const std::vector<const char*>& extensions) const;
        void InitializeDevice(const std::set<uint32_t>& uniqueQueueFamilies, const std::vector<const char*>& extensions);
        void CreateQueue(uint32_t id, VkQueue& queue) const;
        void CreateSwapChain(VkSwapchainCreateInfoKHR& createInfo, VkSwapchainKHR& swapChain,
                std::vector<VkImage>& swapChainImages) const;
        void CreateShaderModule(const VkShaderModuleCreateInfo& createInfo, VkShaderModule& shaderModule);
        void CreatePipelineLayout(const VkPipelineLayoutCreateInfo& createInfo, VkPipelineLayout& pipelineLayout);
        void CreateRenderPass(const VkRenderPassCreateInfo& createInfo, VkRenderPass& renderPass);

        const VkDevice& DeviceHandler() const;

        QueueFamilyIndices FindQueueFamilies(Surface* surface) const;
        SwapChainSupportDetails QuerySwapChainSupport(Surface* surface) const;

        ~PhysicalDevice();
    private:
        bool CheckDeviceExtensionSupport(const std::vector<const char*>& extensions) const;

    private:
        std::optional<VkDevice> m_device;
        VkPhysicalDevice m_physicalDevice{};
        VkPhysicalDeviceProperties m_properties{};
        VkPhysicalDeviceFeatures m_features{};
    };
}

#endif //VAZTERAN_PHYSICALDEVICE_HPP
