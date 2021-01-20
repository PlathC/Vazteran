//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#include "Vazteran/Render/Renderer.hpp"

namespace vzt
{
    Renderer::Renderer(const std::vector<const char*>& extensions)
    {
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
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionsProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsProperties.data());
        std::cout << "available extensions:\n";

        for (const auto& e : extensionsProperties) {
            std::cout << '\t' << e.extensionName << '\n';
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    Renderer::~Renderer()
    {
        vkDestroyInstance(m_vkInstance, nullptr);
    }
}