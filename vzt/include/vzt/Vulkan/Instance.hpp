#ifndef VZT_VULKAN_INSTANCE_HPP
#define VZT_VULKAN_INSTANCE_HPP

#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    class Device;
    class Window;

    namespace val
    {
        constexpr const char* KronosValidation      = "VK_LAYER_KHRONOS_validation";
        constexpr const char* KronosSynchronization = "VK_LAYER_KHRONOS_synchronization2";
        constexpr const char* RenderDocCapture      = "VK_LAYER_RENDERDOC_Capture";
    } // namespace val

    namespace iext
    {
        constexpr const char* DebugUtils = "VK_EXT_debug_utils";
    } // namespace iext

    struct InstanceBuilder
    {
#ifdef NDEBUG
        bool                     enableValidation = false;
        std::vector<const char*> validationLayers;
        std::vector<const char*> extensions;
#else
        bool                     enableValidation   = true;
        std::vector<const char*> validationLayers   = {val::KronosValidation};
        std::vector<const char*> extensions         = {iext::DebugUtils};
#endif
    };

    bool hasValidationLayers(const std::vector<const char*>& validationLayers);
    class Instance
    {
      public:
        Instance(const std::string& name, InstanceBuilder builder = {});
        Instance(Window& window, InstanceBuilder builder = {});

        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        Instance(Instance&&) noexcept;
        Instance& operator=(Instance&&) noexcept;

        ~Instance();

        inline VkInstance                      getHandle() const;
        inline bool                            isValidationEnable() const;
        inline const std::vector<const char*>& getValidationLayers() const;
        inline const std::vector<const char*>& getExtensions() const;

        Device getDevice(DeviceBuilder configuration = {}, View<Surface> surface = {});

      private:
        VkInstance               m_handle         = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

#ifdef NDEBUG
        bool                     m_enableValidation = false;
        std::vector<const char*> m_validationLayers;
        std::vector<const char*> m_extensions;
#else
        bool                     m_enableValidation = true;
        std::vector<const char*> m_validationLayers;
        std::vector<const char*> m_extensions;
#endif
    };
} // namespace vzt

#include "vzt/Vulkan/Instance.inl"

#endif // VZT_VULKAN_INSTANCE_HPP
