#include "vzt/Vulkan/Device.hpp"

#include <array>
#include <set>
#include <unordered_map>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "vzt/Core/Vulkan.hpp"
#include "vzt/Vulkan/Command.hpp"
#include "vzt/Vulkan/Instance.hpp"
#include "vzt/Vulkan/Surface.hpp"
#include "vzt/Vulkan/Swapchain.hpp"

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

    PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) : m_handle(handle)
    {
        vkGetPhysicalDeviceProperties(m_handle, &m_properties);
    }

    bool PhysicalDevice::isSuitable(DeviceBuilder configuration, View<Surface> surface) const
    {
        std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamiliesProperties();

        VkBool32 presentSupport = false;
        for (uint32_t i = 0; i < queueFamilies.size(); i++)
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

    std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamiliesProperties() const
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies{queueFamilyCount};
        vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, queueFamilies.data());

        return queueFamilies;
    }

    bool PhysicalDevice::canQueueFamilyPresent(uint32_t id, View<Surface> surface) const
    {
        const auto queueFamiliesProperties = getQueueFamiliesProperties();

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, id, surface->getHandle(), &presentSupport);

        return presentSupport;
    }

    Format PhysicalDevice::getDepthFormat() const
    {
        // clang-format off
        constexpr std::array<Format, 3> Candidates = {
            vzt::Format::D32SFloat, 
            vzt::Format::D32SFloatS8UInt,
            vzt::Format::D24UNormS8UInt
        };
        // clang-format on

        constexpr VkImageTiling        tiling   = VK_IMAGE_TILING_OPTIMAL;
        constexpr VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        for (vzt::Format format : Candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_handle, toVulkan(format), &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    std::size_t PhysicalDevice::getUniformAlignment(std::size_t alignment)
    {
        const std::size_t minUboAlignment = m_properties.limits.minUniformBufferOffsetAlignment;
        if (minUboAlignment > 0)
            alignment = (alignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
        return alignment;
    }

    Device::Device(View<Instance> instance, PhysicalDevice device, DeviceBuilder configuration, View<Surface> surface)
        : m_instance(instance), m_device(device), m_configuration(configuration)
    {
        std::vector<VkDeviceQueueCreateInfo>    queueCreateInfos{};
        std::unordered_map<QueueType, uint32_t> queueIds{};

        const auto      queuesFamilies = m_device.getQueueFamiliesProperties();
        QueueType       queueTypes     = configuration.queueTypes;
        constexpr float queuePriority  = 1.0f;
        for (uint32_t i = 0; any(queueTypes) && i < queuesFamilies.size(); i++)
        {
            const auto& queueFamily = queuesFamilies[i];

            QueueType selected = QueueType::None;
            if (any(queueTypes & QueueType::Graphics) && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                selected = QueueType::Graphics;
            else if (any(queueTypes & QueueType::Compute) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
                selected = QueueType::Compute;
            else if (any(queueTypes & QueueType::Transfer) && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT))
                selected = QueueType::Transfer;

            if (!any(selected))
                continue;

            queueIds[selected] = i;
            queueTypes         = queueTypes & ~selected;

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

        createInfo.enabledLayerCount = 0;
        if (instance->isValidationEnable())
        {
            const std::vector<const char*>& validationLayers = instance->getValidationLayers();
            createInfo.enabledLayerCount                     = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames                   = validationLayers.data();
        }

        vkCheck(vkCreateDevice(m_device.getHandle(), &createInfo, nullptr, &m_handle),
                "Failed to create logical device.");

        queueTypes = configuration.queueTypes;

        uint32_t presentQueueId = ~0;
        for (auto [type, id] : queueIds)
            m_queues.emplace(this, type, id, m_device.canQueueFamilyPresent(id, surface));

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
        if (m_handle == VK_NULL_HANDLE)
            return;

        wait();

        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_handle, nullptr);
    }

    void Device::wait() const { vkDeviceWaitIdle(m_handle); }

    std::vector<View<Queue>> Device::getQueues() const
    {
        std::vector<View<Queue>> queues{};
        for (const auto& queue : m_queues)
            queues.emplace_back(queue);

        return queues;
    }

    View<Queue> Device::getQueue(QueueType type) const
    {
        assert(any(m_configuration.queueTypes & type) &&
               "This device has not been configured with the requested queue.");

        for (const auto& queue : m_queues)
        {
            if (queue.getType() == type)
                return queue;
        }

        return {};
    }

    View<Queue> Device::getPresentQueue() const
    {
        for (const auto& queue : m_queues)
        {
            if (queue.canPresent())
                return queue;
        }

        return {};
    }

    Queue::Queue(View<Device> device, QueueType type, uint32_t id, bool canPresent)
        : m_device(device), m_type(type), m_id(id), m_canPresent(canPresent)
    {
        vkGetDeviceQueue(device->getHandle(), id, 0, &m_handle);
    }

    void Queue::oneShot(const SingleTimeCommandFunction& function) const
    {
        CommandPool pool{m_device, this, 1};
        pool.allocateCommandBuffers(1);

        CommandBuffer commands = pool[0];
        function(commands);
        submit(commands);
    }

    void Queue::submit(CommandBuffer& commandBuffer, const SwapchainSubmission& submission) const
    {
        assert(m_canPresent && "This queue is unable to present and is used for a swapchain submission");

        commandBuffer.flush();
        const VkCommandBuffer commands = commandBuffer.getHandle();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        constexpr VkPipelineStageFlags waitStage =
            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.pWaitDstStageMask    = &waitStage;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = &submission.imageAvailable;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = &submission.renderComplete;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commands;

        vkResetFences(m_device->getHandle(), 1, &submission.frameComplete);
        vkCheck(vkQueueSubmit(m_handle, 1, &submitInfo, submission.frameComplete),
                "Failed to submit swapchain submission");
    }

    void Queue::submit(CommandBuffer& commandBuffer) const
    {
        commandBuffer.flush();
        const VkCommandBuffer commands = commandBuffer.getHandle();

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commands;

        vkQueueSubmit(m_handle, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_handle);
    }

} // namespace vzt