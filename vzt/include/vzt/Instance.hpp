#ifndef VZT_INSTANCE_HPP
#define VZT_INSTANCE_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "vzt/Device.hpp"

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

    struct InstanceConfiguration
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

    bool hasValidationLayers(const std::vector<const char*>& validationLayers);
    class Instance
    {
      public:
        Instance(const std::string& name, InstanceConfiguration configuration = {});
        Instance(Window& window, InstanceConfiguration configuration = {});

        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        Instance(Instance&&) noexcept;
        Instance& operator=(Instance&&) noexcept;

        ~Instance();

        inline VkInstance                   getHandle() const;
        inline const InstanceConfiguration& getConfiguration() const;
        Device getDevice(DeviceConfiguration configuration = {}, View<Surface> surface = {});

      private:
        VkInstance               m_handle         = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        InstanceConfiguration    m_configuration;
    };
} // namespace vzt

#include "vzt/Instance.inl"

#endif // VZT_INSTANCE_HPP
