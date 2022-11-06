#include "vzt/Instance.hpp"

#include <iostream>
#include <stdexcept>

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "vzt/Core/Logger.hpp"
#include "vzt/Window.hpp"

namespace vzt
{
    bool checkConfiguration(const Configuration& configuration)
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const std::string& layerName : configuration.validationLayers)
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

    Instance::Instance(const std::string& name, Configuration configuration)
    {
        if (configuration.enableValidation && !checkConfiguration(configuration))
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
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(configuration.extensions.size());
        createInfo.ppEnabledExtensionNames = configuration.extensions.data();

        createInfo.enabledLayerCount = 0;

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreate;
        if (configuration.enableValidation)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(configuration.validationLayers.size());
            createInfo.ppEnabledLayerNames = configuration.validationLayers.data();

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

        if (!configuration.enableValidation)
            return;

        auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_handle, "vkCreateDebugUtilsMessengerEXT"));

        if (!vkCreateDebugUtilsMessengerEXT)
            logger::error("Failed to create debug messenger");

        vkCheck(vkCreateDebugUtilsMessengerEXT(m_handle, &debugMessengerCreate, nullptr, &m_debugMessenger),
                "Failed to create debug messenger");
    }

    Instance::Instance(Window& window, Configuration configuration)
        : Instance(std::string(window.getTitle()), window.getConfiguration(configuration))
    {
    }

    Instance::Instance(Instance&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_devices, other.m_devices);
    }

    Instance& Instance::operator=(Instance&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_devices, other.m_devices);

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
        {
            vkDestroyDebugUtilsMessengerEXT(m_handle, m_debugMessenger, nullptr);
        }

        vkDestroyInstance(m_handle, nullptr);
    }

    Surface::Surface(const Window& window, const Instance& instance) : m_instance(&instance)
    {
        if (!SDL_Vulkan_CreateSurface(window.getHandle(), instance.getHandle(), &m_handle))
            logger::error("[SDL] {}", SDL_GetError());
    }

    Surface::Surface(Surface&& other) noexcept
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_handle, other.m_handle);
    }

    Surface& Surface::operator=(Surface&& other) noexcept
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_handle, other.m_handle);

        return *this;
    }

    Surface::~Surface()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_instance->getHandle(), m_handle, nullptr);
    }

} // namespace vzt
