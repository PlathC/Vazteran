//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#include "Vazteran/Render/Vulkan/ErrorHandler.hpp"

namespace vzt
{
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
        );
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator)
    {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
        );

        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    DebugMessenger::DebugMessenger()
    {

        m_creationInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        m_creationInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                       | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                       | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        m_creationInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        m_creationInfo.pfnUserCallback = &DebugMessenger::DebugCallback;

        // TODO: Give pointer on logger
        m_creationInfo.pUserData = nullptr;
    }

    VkDebugUtilsMessengerCreateInfoEXT& DebugMessenger::CreationInfo()
    {
        return m_creationInfo;
    }

    void DebugMessenger::Initialize(std::shared_ptr<VkInstance> vkInstance)
    {
        m_vkInstance = std::move(vkInstance);
        if (CreateDebugUtilsMessengerEXT(*m_vkInstance, &m_creationInfo, nullptr, &m_messenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    VkBool32 DebugMessenger::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                           void *pUserData)
    {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cerr << "[FATAL] validation layer: " << pCallbackData->pMessage << std::endl;
        }
        else
        {
            std::cerr << "[Warning] validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    DebugMessenger::~DebugMessenger()
    {
        DestroyDebugUtilsMessengerEXT(*m_vkInstance, m_messenger, nullptr);
    }
}
