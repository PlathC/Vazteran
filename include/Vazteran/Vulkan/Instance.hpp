#ifndef VAZTERAN_INSTANCE_HPP
#define VAZTERAN_INSTANCE_HPP

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace vzt {
    class Instance {
    public:
        Instance(std::string_view name, std::vector<const char*> extensions);
        VkInstance Get() { return m_instance; };
        ~Instance();

#ifdef NDEBUG
        static constexpr bool EnableValidationLayers = false;
#else
        static constexpr bool EnableValidationLayers = true;
#endif
        static const std::vector<const char*> ValidationLayers;
    private:
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;

        static bool CheckValidationLayerSupport();
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
}

#endif //VAZTERAN_INSTANCE_HPP
