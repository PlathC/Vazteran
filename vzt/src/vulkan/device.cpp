#include "vzt/vulkan/device.hpp"

#include <algorithm>
#include <unordered_map>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define VOLK_IMPLEMENTATION
#include <stdexcept>

#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include "vzt/core/logger.hpp"
#include "vzt/vulkan/command.hpp"
#include "vzt/vulkan/instance.hpp"
#include "vzt/vulkan/surface.hpp"
#include "vzt/vulkan/swapchain.hpp"

namespace vzt
{
    // Returns the index of the first requested field which is not supported, or DeviceFeatures::NoField
    static uint32_t firstMissing(const VkBool32* requested, const VkBool32* supported, std::size_t count)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            if (requested[i] && !supported[i])
                return i;
        }

        return DeviceFeatures::NoField;
    }

    // Compares the VkBool32 fields following the header (sType, pNext) of two structures of the same type
    static uint32_t firstMissing(const void* requested, const void* supported, std::size_t size)
    {
        constexpr std::size_t Header = sizeof(VkBaseOutStructure);
        const auto            fields = [](const void* s) {
            return reinterpret_cast<const VkBool32*>(static_cast<const std::byte*>(s) + Header);
        };
        return firstMissing(fields(requested), fields(supported), (size - Header) / sizeof(VkBool32));
    }

    std::string DeviceFeatures::findMissing(const DeviceFeatures& supported) const
    {
        std::string missing;
        const auto  check = [&](std::string_view structure, uint32_t field) {
            if (missing.empty() && field != NoField)
                missing = fmt::format("field {} of {}", field, structure);
        };

        check("core", firstMissing(&core.robustBufferAccess, &supported.core.robustBufferAccess,
                                   sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32)));
        check(string_VkStructureType(v11.sType), firstMissing(&v11, &supported.v11, sizeof(v11)));
        check(string_VkStructureType(v12.sType), firstMissing(&v12, &supported.v12, sizeof(v12)));

        for (const std::vector<std::byte>& other : m_others)
        {
            const VkStructureType type = reinterpret_cast<const VkBaseOutStructure*>(other.data())->sType;
            const auto counterpart     = std::ranges::find_if(supported.m_others, [&](const std::vector<std::byte>& s) {
                return reinterpret_cast<const VkBaseOutStructure*>(s.data())->sType == type;
            });
            check(string_VkStructureType(type), counterpart != supported.m_others.end()
                                                    ? firstMissing(other.data(), counterpart->data(), other.size())
                                                    : 0);
        }

        return missing;
    }

    VkPhysicalDeviceFeatures2& DeviceFeatures::chain()
    {
        m_head.features = core;
        m_head.pNext    = &v11;
        v11.pNext       = &v12;

        auto* last = reinterpret_cast<VkBaseOutStructure*>(&v12);
        for (std::vector<std::byte>& other : m_others)
        {
            last->pNext = reinterpret_cast<VkBaseOutStructure*>(other.data());
            last        = last->pNext;
        }
        last->pNext = nullptr;

        return m_head;
    }

    DeviceBuilder DeviceBuilder::offline()
    {
        DeviceBuilder builder{};
        builder.m_queueTypes                         = QueueType::Graphics | QueueType::Compute | QueueType::Transfer;
        builder.m_features.core.samplerAnisotropy    = VK_TRUE;
        builder.m_features.core.shaderResourceMinLod = VK_TRUE;

        builder.require(cap::DynamicRendering).require(cap::BufferDeviceAddress).require(cap::ShaderDebugInfo);
        builder.request(cap::PortabilitySubset);

        return builder;
    }

    DeviceBuilder DeviceBuilder::standard() { return offline().require(cap::Swapchain); }

    DeviceBuilder DeviceBuilder::rt()
    {
        DeviceBuilder builder                     = standard().require(cap::RayQuery).require(cap::RayTracing);
        builder.m_features.v12.descriptorIndexing = VK_TRUE;

        return builder;
    }

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
    { vkGetPhysicalDeviceProperties(m_handle, &m_properties); }

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

        const auto reject = [&](std::string_view reason) {
            logger::warn("[{}] Unsuitable device: {}", m_properties.deviceName, reason);
            return false;
        };

        if (any(requestedTypes))
            return reject("missing queue types");

        if (surface && (!presentSupport || !hasSwapchain(m_handle, surface->getHandle())))
            return reject("can't present to the surface");

        if (const auto missing = getMissingExtensions(configuration.getExtensions()); !missing.empty())
            return reject(fmt::format("missing extension {}", *missing.begin()));

        const DeviceFeatures& requested = configuration.getDeviceFeatures();
        if (const std::string missing = requested.findMissing(getFeatures(requested)); !missing.empty())
            return reject(fmt::format("unsupported feature ({})", missing));

        for (const Capability& capability : configuration.getRequiredCapabilities())
        {
            if (!supports(capability))
                return reject(fmt::format("missing capability {}", capability.name));
        }

        return true;
    }

    bool PhysicalDevice::supports(const Capability& capability) const
    {
        DeviceFeatures features;
        if (capability.enable)
            capability.enable(features);

        return hasExtensions(capability.extensions) && features.findMissing(getFeatures(features)).empty();
    }

    DeviceFeatures PhysicalDevice::getFeatures(DeviceFeatures features) const
    {
        // The driver overwrites every field of the structures it knows
        VkPhysicalDeviceFeatures2& head = features.chain();
        vkGetPhysicalDeviceFeatures2(m_handle, &head);
        features.core = head.features;

        return features;
    }

    bool PhysicalDevice::hasExtensions(const std::vector<const char*>& extensions) const
    { return getMissingExtensions(extensions).empty(); }

    std::set<std::string> PhysicalDevice::getMissingExtensions(const std::vector<const char*>& extensions) const
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> missingExtensions{extensions.begin(), extensions.end()};
        for (const auto& extension : availableExtensions)
            missingExtensions.erase(extension.extensionName);

        return missingExtensions;
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
        : m_instance(instance), m_device(device)
    {
        const auto enable = [&](const Capability& capability) {
            for (dext::Extension extension : capability.extensions)
                configuration.add(extension);

            if (capability.enable)
                capability.enable(configuration.getDeviceFeatures());

            m_capabilities.emplace_back(capability.name);
        };

        for (const Capability& capability : configuration.getRequiredCapabilities())
            enable(capability);

        for (const Capability& capability : configuration.getRequestedCapabilities())
        {
            if (m_device.supports(capability))
                enable(capability);
        }
        m_configuration = configuration;

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
        createInfo.pNext                = &configuration.getDeviceFeatures().chain();

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

        const bool testPresentation = m_device.hasExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME}) &&
                                      m_instance->hasExtension(VK_KHR_SURFACE_EXTENSION_NAME);
        for (auto [type, id] : queueIds)
        {
            bool canPresent = false;
            if (testPresentation)
                canPresent = m_device.canQueueFamilyPresent(id, surface);

            m_queues.emplace(this, type, id, canPresent);
        }

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice         = m_device.getHandle();
        allocatorInfo.device                 = m_handle;
        allocatorInfo.instance               = instance->getHandle();
        allocatorInfo.vulkanApiVersion       = toVulkan(instance->getAPIVersion());

        if (configuration.getDeviceFeatures().v12.bufferDeviceAddress)
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        if (std::ranges::find(extensions, std::string_view(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) != extensions.end())
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

        // To use VMA_DYNAMIC_VULKAN_FUNCTIONS in new versions of VMA you now have to pass
        // VmaVulkanFunctions::vkGetInstanceProcAddr and vkGetDeviceProcAddr as
        // VmaAllocatorCreateInfo::pVulkanFunctions. Other members can be null.
        VmaVulkanFunctions vmaVulkanFunctions{};
        vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vmaVulkanFunctions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;
        allocatorInfo.pVulkanFunctions           = &vmaVulkanFunctions;

        vkCheck(vmaCreateAllocator(&allocatorInfo, &m_allocator), "Failed to create allocator.");
    }

    Device::Device(Device&& other) noexcept
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_device, other.m_device);
        std::swap(m_table, other.m_table);

        std::swap(m_handle, other.m_handle);
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_configuration, other.m_configuration);
        std::swap(m_capabilities, other.m_capabilities);

        for (auto& queue : other.m_queues)
            m_queues.emplace(this, queue.getType(), queue.getId(), queue.canPresent());
    }

    Device& Device::operator=(Device&& other) noexcept
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_device, other.m_device);
        std::swap(m_table, other.m_table);

        std::swap(m_handle, other.m_handle);
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_configuration, other.m_configuration);
        std::swap(m_capabilities, other.m_capabilities);

        for (auto& queue : other.m_queues)
            m_queues.emplace(this, queue.getType(), queue.getId(), queue.canPresent());
        return *this;
        ;
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

    bool Device::has(const Capability& capability) const
    {
        return std::ranges::find(m_capabilities, capability.name) != m_capabilities.end(); //
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
