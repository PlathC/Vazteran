#ifndef VAZTERAN_INSTANCE_HPP
#define VAZTERAN_INSTANCE_HPP

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace vzt {
    class Instance {
    public:
        Instance(std::string_view name, std::vector<const char*> extensions,
                 const std::vector<const char*>& validationLayers = DefaultValidationLayers);

        std::vector<VkPhysicalDevice> EnumeratePhysicalDevice();
        VkInstance VkHandle() const { return m_handle; };
        std::vector<const char*> ValidationLayers() const { return m_validationLayers; }

        ~Instance();

#ifdef NDEBUG
        static constexpr bool EnableValidationLayers = false;
#else
        static constexpr bool EnableValidationLayers = true;
#endif
        static const std::vector<const char*> DefaultValidationLayers;
    private:
        static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);

        VkInstance m_handle;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        std::vector<const char*> m_validationLayers;
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
}

#endif //VAZTERAN_INSTANCE_HPP
