#include "vzt/Vulkan/Instance.hpp"

#include "vzt/Core/Logger.hpp"
#include "vzt/Core/Vulkan.hpp"
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
        : m_enableValidation(std::move(builder.enableValidation)),
          m_validationLayers(std::move(builder.validationLayers)), m_extensions(std::move(builder.extensions))
    {
        if (m_enableValidation && !hasValidationLayers(m_validationLayers))
            logger::error("Instance configuration validation failed");

        VkApplicationInfo appInfo{};
        appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = name.c_str();
        appInfo.pEngineName      = "Vazteran";
        appInfo.engineVersion    = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion       = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_extensions.size());
        createInfo.ppEnabledExtensionNames = m_extensions.data();

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

        if (!m_enableValidation)
            return;

        auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_handle, "vkCreateDebugUtilsMessengerEXT"));

        if (!vkCreateDebugUtilsMessengerEXT)
            logger::error("Failed to create debug messenger");

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

        const auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_handle, "vkDestroyDebugUtilsMessengerEXT"));
        if (vkDestroyDebugUtilsMessengerEXT != nullptr && m_handle != VK_NULL_HANDLE &&
            m_debugMessenger != VK_NULL_HANDLE)
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

        for (uint32_t i = 0; i < deviceCount; i++)
        {
            const auto device = PhysicalDevice(devices[i]);
            if (device.isSuitable(configuration, surface))
                return Device(this, device, configuration, surface);
        }

        logger::error("Can't find suitable device, defaulting.");
        return Device(this, PhysicalDevice(devices[0]), configuration, surface);
    }

} // namespace vzt
