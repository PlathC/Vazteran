#ifndef VZT_VULKAN_INSTANCE_HPP
#define VZT_VULKAN_INSTANCE_HPP

#include <string>

#include "vzt/vulkan/device.hpp"

namespace vzt
{
    class Window;

    namespace val
    {
        constexpr const char* KronosValidation      = "VK_LAYER_KHRONOS_validation";
        constexpr const char* KronosSynchronization = "VK_LAYER_KHRONOS_synchronization2";
        constexpr const char* RenderDocCapture      = "VK_LAYER_RENDERDOC_Capture";
    } // namespace val

    namespace iext
    {
        constexpr const char* DebugUtils                = "VK_EXT_debug_utils";
        constexpr const char* PhysicalDeviceProperties2 = "VK_KHR_get_physical_device_properties2";
        constexpr const char* PortabilityEnumeration    = "VK_KHR_portability_enumeration";
    } // namespace iext

    enum class VulkanVersion : uint32_t
    {
        V1_0 = VK_API_VERSION_1_0,
        V1_1 = VK_API_VERSION_1_1,
        V1_2 = VK_API_VERSION_1_2,
        V1_3 = VK_API_VERSION_1_3
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(VulkanVersion, uint32_t)

    inline uint32_t toVersion(uint8_t major, uint8_t minor, uint8_t patch);

    struct InstanceBuilder
    {
        uint32_t    engineVersion = 0;
        std::string engineName;

#ifdef __APPLE__ // MoltenVK is currently targeting Vulkan 1.2
        VulkanVersion apiVersion = VulkanVersion::V1_2;
#else
        VulkanVersion apiVersion = VulkanVersion::V1_3;
#endif

#ifdef NDEBUG
        bool                     enableValidation = false;
        std::vector<const char*> validationLayers;
        std::vector<const char*> extensions = {iext::PortabilityEnumeration, iext::PhysicalDeviceProperties2};
#else
        bool                     enableValidation = true;
        std::vector<const char*> validationLayers = {val::KronosValidation};
        std::vector<const char*> extensions       = {iext::DebugUtils, iext::PortabilityEnumeration,
                                                     iext::PhysicalDeviceProperties2};
#endif

#ifdef __APPLE__
        bool enablePortability = true;
#else
        bool enablePortability = false;
#endif
    };

    bool hasValidationLayers(const std::vector<const char*>& validationLayers);
    class Instance
    {
      public:
        Instance(const std::string& name, InstanceBuilder builder = {});

        Instance(const Instance&)            = delete;
        Instance& operator=(const Instance&) = delete;

        Instance(Instance&&) noexcept;
        Instance& operator=(Instance&&) noexcept;

        ~Instance();

        inline VkInstance                      getHandle() const;
        inline VulkanVersion                   getAPIVersion() const;
        inline bool                            isValidationEnable() const;
        inline const std::vector<const char*>& getValidationLayers() const;
        inline const std::vector<const char*>& getExtensions() const;

        Device getDevice(DeviceBuilder configuration = {}, View<Surface> surface = {});

      private:
        VkInstance               m_handle         = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

        VulkanVersion m_version = VulkanVersion::V1_3;

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

#include "vzt/vulkan/instance.inl"

#endif // VZT_VULKAN_INSTANCE_HPP
