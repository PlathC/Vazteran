#ifndef VZT_INSTANCE_HPP
#define VZT_INSTANCE_HPP

#include <optional>
#include <string>
#include <vector>

#include "vzt/Core/Vulkan.hpp"

namespace vzt
{
    class Device;

    namespace validation
    {
        constexpr const char* KronosValidation      = "VK_LAYER_KHRONOS_validation";
        constexpr const char* KronosSynchronization = "VK_LAYER_KHRONOS_synchronization2";
        constexpr const char* RenderDocCapture      = "VK_LAYER_RENDERDOC_Capture";
    } // namespace validation

    namespace extension
    {
        constexpr const char* VkDebugUtils = "VK_EXT_debug_utils";
    } // namespace extension

    struct Configuration
    {
#ifdef NDEBUG
        bool                     enableValidation = false;
        std::vector<const char*> validationLayers;
        std::vector<const char*> extensions;
#else
        bool                     enableValidation = true;
        std::vector<const char*> validationLayers = {validation::KronosValidation};
        std::vector<const char*> extensions       = {extension::VkDebugUtils};
#endif
    };

    class Instance
    {
      public:
        Instance(const std::string& name, Configuration configuration = {});

        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        Instance(Instance&&);
        Instance& operator=(Instance&&);

        ~Instance();

        Device getDevice(std::optional<uint32_t> id = {});

      private:
        VkInstance               m_handle         = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

        struct DeviceHandle
        {
        };
        std::vector<DeviceHandle> m_devices;
    };

    class Device
    {
    };
} // namespace vzt

#endif // VZT_INSTANCE_HPP
