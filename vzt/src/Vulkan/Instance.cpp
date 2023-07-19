#include "vzt/Vulkan/Instance.hpp"

#include <stdexcept>

#include "vzt/Core/Logger.hpp"
#include "vzt/Window.hpp"

namespace vzt
{
    bool hasValidationLayers(const std::vector<const char*>& validationLayers)
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const std::string& layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers)
            {
                if (layerName != layerProperties.layerName)
                    continue;

                layerFound = true;
                break;
            }

            if (!layerFound)
                return false;
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void*                                       pUserData)
    {

        // TODO: Handle this in a real logger
        std::string severityDisplay;
        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            logger::info("[VULKAN] {}", pCallbackData->pMessage);
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            logger::info("[VULKAN] {}", pCallbackData->pMessage);
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            logger::warn("[VULKAN] {}", pCallbackData->pMessage);
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            logger::debug("[VULKAN] {}", pCallbackData->pMessage);

        return VK_FALSE;
    }

    Instance::Instance(const std::string& name, InstanceBuilder builder)
        : m_version(builder.apiVersion), m_enableValidation(std::move(builder.enableValidation)),
          m_validationLayers(std::move(builder.validationLayers)), m_extensions(std::move(builder.extensions))
    {
        if (volkInitialize() != VK_SUCCESS)
            throw std::runtime_error("Failed to find Vulkan.");

        if (m_enableValidation && !hasValidationLayers(m_validationLayers))
            logger::error("Instance configuration validation failed");

        VkApplicationInfo appInfo{};
        appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = name.c_str();
        appInfo.pEngineName      = builder.engineName.c_str();
        appInfo.engineVersion    = builder.engineVersion;
        appInfo.apiVersion       = toVulkan(builder.apiVersion);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_extensions.size());
        createInfo.ppEnabledExtensionNames = m_extensions.data();

        if (builder.enablePortability)
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        createInfo.enabledLayerCount = 0;

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreate;
        if (m_enableValidation)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(m_validationLayers.size());
            createInfo.ppEnabledLayerNames = m_validationLayers.data();

            debugMessengerCreate                 = {};
            debugMessengerCreate.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugMessengerCreate.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugMessengerCreate.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugMessengerCreate.pfnUserCallback = debugCallback;

            createInfo.pNext = &debugMessengerCreate;
        }

        vkCheck(vkCreateInstance(&createInfo, nullptr, &m_handle), "Failed to create Vulkan instance");
        volkLoadInstance(m_handle);

        if (!m_enableValidation)
            return;

        vkCheck(vkCreateDebugUtilsMessengerEXT(m_handle, &debugMessengerCreate, nullptr, &m_debugMessenger),
                "Failed to create debug messenger");
    }

    Instance::Instance(Window& window, InstanceBuilder builder)
        : Instance(std::string(window.getTitle()), window.getConfiguration(std::move(builder)))
    {
    }

    Instance::Instance(Instance&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_debugMessenger, other.m_debugMessenger);
        std::swap(m_enableValidation, other.m_enableValidation);
        std::swap(m_validationLayers, other.m_validationLayers);
        std::swap(m_extensions, other.m_extensions);
    }

    Instance& Instance::operator=(Instance&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_debugMessenger, other.m_debugMessenger);
        std::swap(m_enableValidation, other.m_enableValidation);
        std::swap(m_validationLayers, other.m_validationLayers);
        std::swap(m_extensions, other.m_extensions);

        return *this;
    }

    Instance::~Instance()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        if (m_debugMessenger != VK_NULL_HANDLE)
            vkDestroyDebugUtilsMessengerEXT(m_handle, m_debugMessenger, nullptr);

        vkDestroyInstance(m_handle, nullptr);
    }

    Device Instance::getDevice(DeviceBuilder configuration, View<Surface> surface)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_handle, &deviceCount, nullptr);
        if (deviceCount == 0)
            logger::error("Failed to find GPUs with Vulkan support!");

        auto devices = std::vector<VkPhysicalDevice>(deviceCount);
        vkEnumeratePhysicalDevices(m_handle, &deviceCount, devices.data());

        uint32_t selectedDevice = 0;
        bool isDiscrete = false;
        for (uint32_t i = 0; i < deviceCount; i++)
        {
            const auto device = PhysicalDevice(devices[i]);
            if (device.isSuitable(configuration, surface))
                selectedDevice = i;

            if(!isDiscrete && device.getProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                selectedDevice = i;
                isDiscrete = true;
            }
        }

        return {this, PhysicalDevice(devices[selectedDevice]), configuration, surface};
    }

} // namespace vzt
