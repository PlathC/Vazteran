#include "vzt/Vulkan/Device.hpp"

#include <unordered_map>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include <stdexcept>

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

        QueueType requestedTypes = configuration.getQueueTypes();
        for (uint32_t i = 0; i < queueFamilies.size(); i++)
        {
            const auto& queueFamily = queueFamilies[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                requestedTypes = remove(requestedTypes, QueueType::Graphics);

            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                requestedTypes = remove(requestedTypes, QueueType::Compute);

            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                requestedTypes = remove(requestedTypes, QueueType::Transfer);

            if (!surface || presentSupport)
                continue;

            vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, i, surface->getHandle(), &presentSupport);
        }

        bool isSuitable = !any(requestedTypes) && hasExtensions(configuration.getExtensions());
        if (surface)
            isSuitable &= hasSwapchain(m_handle, surface->getHandle());

        const auto& requestedDeviceFeatures   = configuration.getDeviceFeatures();
        const auto  requestedPhysicalFeatures = requestedDeviceFeatures.getPhysicalFeatures();

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(m_handle, &supportedFeatures);
        isSuitable &= GenericDeviceFeature::match(requestedPhysicalFeatures, GenericDeviceFeature(supportedFeatures));

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
        for (Format format : Candidates)
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

    std::size_t PhysicalDevice::getUniformAlignment(std::size_t alignment) const
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
        QueueType       queueTypes     = configuration.getQueueTypes();
        constexpr float queuePriority  = 1.0f;
        for (uint32_t i = 0; any(queueTypes) && i < queuesFamilies.size(); i++)
        {
            auto queueFamily = queuesFamilies[i];

            QueueType selected = QueueType::None;
            while (queueFamily.queueFlags)
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    selected |= QueueType::Graphics;
                    queueFamily.queueFlags &= static_cast<VkQueueFlags>(~VK_QUEUE_GRAPHICS_BIT);
                }
                else if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    selected |= QueueType::Compute;
                    queueFamily.queueFlags &= static_cast<VkQueueFlags>(~VK_QUEUE_COMPUTE_BIT);
                }
                else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    selected |= QueueType::Transfer;
                    queueFamily.queueFlags &= static_cast<VkQueueFlags>(~VK_QUEUE_TRANSFER_BIT);
                }
                else // Unsupported queue family
                {
                    break;
                }
            }

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

        VkDeviceCreateInfo createInfo{};
        createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos    = queueCreateInfos.data();
        createInfo.pEnabledFeatures     = nullptr;
        createInfo.pNext                = &configuration.getDeviceFeatures().getAllFeatures();

        const auto& extensions             = configuration.getExtensions();
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        createInfo.enabledLayerCount = 0;
        if (instance->isValidationEnable())
        {
            const std::vector<const char*>& validationLayers = instance->getValidationLayers();
            createInfo.enabledLayerCount                     = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames                   = validationLayers.data();
        }

        vkCheck(vkCreateDevice(m_device.getHandle(), &createInfo, nullptr, &m_handle),
                "Failed to create logical device.");
        volkLoadDeviceTable(&m_table, m_handle);

        queueTypes = configuration.getQueueTypes();
        for (auto [type, id] : queueIds)
            m_queues.emplace(this, type, id, m_device.canQueueFamilyPresent(id, surface));

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice         = m_device.getHandle();
        allocatorInfo.device                 = m_handle;
        allocatorInfo.instance               = instance->getHandle();
        allocatorInfo.vulkanApiVersion       = toVulkan(instance->getAPIVersion());
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        // To use VMA_DYNAMIC_VULKAN_FUNCTIONS in new versions of VMA you now have to pass
        // VmaVulkanFunctions::vkGetInstanceProcAddr and vkGetDeviceProcAddr as
        // VmaAllocatorCreateInfo::pVulkanFunctions. Other members can be null.
        VmaVulkanFunctions vmaVulkanFunctions{};
        vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vmaVulkanFunctions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;
        allocatorInfo.pVulkanFunctions           = &vmaVulkanFunctions;

        vkCheck(vmaCreateAllocator(&allocatorInfo, &m_allocator), "Failed to create allocator.");
    }

    Device::Device(Device&& other) noexcept : m_device(std::move(other.m_device))
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_table, other.m_table);
        std::swap(m_handle, other.m_handle);
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_configuration, other.m_configuration);
        std::swap(m_queues, other.m_queues);
    }

    Device& Device::operator=(Device&& other) noexcept
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_device, other.m_device);
        std::swap(m_table, other.m_table);
        std::swap(m_handle, other.m_handle);
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_configuration, other.m_configuration);
        std::swap(m_queues, other.m_queues);

        return *this;
    }

    Device::~Device()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        wait();

        vmaDestroyAllocator(m_allocator);
        m_table.vkDestroyDevice(m_handle, nullptr);
    }

    void Device::wait() const { m_table.vkDeviceWaitIdle(m_handle); }

    std::vector<View<Queue>> Device::getQueues() const
    {
        std::vector<View<Queue>> queues{};
        for (const auto& queue : m_queues)
            queues.emplace_back(queue);

        return queues;
    }

    View<Queue> Device::getQueue(QueueType type) const
    {
        assert(any(m_configuration.getQueueTypes() & type) &&
               "This device has not been configured with the requested queue.");

        for (const auto& queue : m_queues)
        {
            if (any(queue.getType() & type))
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
        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkGetDeviceQueue(device->getHandle(), id, 0, &m_handle);
    }

    void Queue::oneShot(const SingleTimeCommandFunction& function) const
    {
        CommandPool pool{m_device, this, 1};
        pool.allocateCommandBuffers(1);

        CommandBuffer commands = pool[0];

        commands.begin();
        function(commands);
        commands.end();

        submit(commands);
    }

    void Queue::submit(const CommandBuffer& commandBuffer, const SwapchainSubmission& submission) const
    {
        assert(m_canPresent && "This queue is unable to present and is used for a swapchain submission");

        const VkCommandBuffer commands = commandBuffer.getHandle();

        VkSubmitInfo submitInfo{};
        submitInfo.sType                         = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        constexpr VkPipelineStageFlags waitStage = toVulkan(PipelineStage::ColorAttachmentOutput);
        submitInfo.pWaitDstStageMask             = &waitStage;
        submitInfo.waitSemaphoreCount            = 1;
        submitInfo.pWaitSemaphores               = &submission.imageAvailable;
        submitInfo.signalSemaphoreCount          = 1;
        submitInfo.pSignalSemaphores             = &submission.renderComplete;
        submitInfo.commandBufferCount            = 1;
        submitInfo.pCommandBuffers               = &commands;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkResetFences(m_device->getHandle(), 1, &submission.frameComplete);
        vkCheck(table.vkQueueSubmit(m_handle, 1, &submitInfo, submission.frameComplete),
                "Failed to submit swapchain submission");
    }

    void Queue::submit(const CommandBuffer& commandBuffer) const
    {
        const VkCommandBuffer commands = commandBuffer.getHandle();

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commands;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkQueueSubmit(m_handle, 1, &submitInfo, VK_NULL_HANDLE);
        table.vkQueueWaitIdle(m_handle);
    }
} // namespace vzt
