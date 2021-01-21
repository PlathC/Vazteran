//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#ifndef VAZTERAN_ERRORHANDLER_HPP
#define VAZTERAN_ERRORHANDLER_HPP

#include <iostream>
#include <stdexcept>

#include <vulkan/vulkan.h>

namespace vzt
{
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator);

    class DebugMessenger
    {
    public:
        DebugMessenger();
        VkDebugUtilsMessengerCreateInfoEXT& CreationInfo();
        void Initialize(std::shared_ptr<VkInstance> vkInstance);
        ~DebugMessenger();
    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData);

        std::shared_ptr<VkInstance> m_vkInstance;
        VkDebugUtilsMessengerCreateInfoEXT m_creationInfo{};
        VkDebugUtilsMessengerEXT m_messenger;
    };
}

#endif //VAZTERAN_ERRORHANDLER_HPP
