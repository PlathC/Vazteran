#ifndef VZT_INSTANCE_HPP
#define VZT_INSTANCE_HPP

#include <optional>
#include <string>
#include <vector>

#include "vzt/Core/Vulkan.hpp"

namespace vzt
{
    class Device;
    class Window;

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
        Instance(Window& window, Configuration configuration = {});

        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        Instance(Instance&&) noexcept;
        Instance& operator=(Instance&&) noexcept;

        ~Instance();

        inline VkInstance getHandle() const;
        Device            getDevice();
        Device            getDevice(VkSurfaceKHR surface);

      private:
        VkInstance               m_handle         = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

        struct DeviceHandle
        {
        };
        std::vector<DeviceHandle> m_devices;
    };

    class Surface
    {
      public:
        Surface(const Window& window, const Instance& instance);

        Surface(const Surface&)            = delete;
        Surface& operator=(const Surface&) = delete;

        Surface(Surface&&) noexcept;
        Surface& operator=(Surface&&) noexcept;

        ~Surface();

      private:
        const Instance* m_instance = nullptr;
        VkSurfaceKHR    m_handle   = VK_NULL_HANDLE;
    };

    class Device
    {
    };
} // namespace vzt

#include "vzt/Instance.inl"

#endif // VZT_INSTANCE_HPP
