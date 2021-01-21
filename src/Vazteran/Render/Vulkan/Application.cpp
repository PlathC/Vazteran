//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#include "Vazteran/Render/Vulkan/Application.hpp"

namespace vzt
{
    Application::Application(std::vector<const char*> extensions, SurfaceInitializer surfaceInitializer)
    {
        if (enableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vazteran";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vazteran";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if constexpr (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            createInfo.ppEnabledLayerNames = ValidationLayers.data();
            m_debugMessenger = std::make_unique<DebugMessenger>();
            createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&m_debugMessenger->CreationInfo());
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        m_vkInstance = std::shared_ptr<VkInstance>(new VkInstance(), [](VkInstance* instance)
        {
            vkDestroyInstance(*instance, nullptr);
        });

        if (vkCreateInstance(&createInfo, nullptr, m_vkInstance.get()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }

        if constexpr (enableValidationLayers)
        {
            m_debugMessenger->Initialize(m_vkInstance);
        }


        m_surface = std::make_unique<Surface>(m_vkInstance, surfaceInitializer);

        m_deviceManager = std::make_unique<DeviceManager>(m_vkInstance, m_surface.get());
    }

    Application::~Application()
    {
    }

    bool Application::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const auto& layerName : ValidationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers)
            {
                if (std::string_view(layerName) == layerProperties.layerName)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }
}