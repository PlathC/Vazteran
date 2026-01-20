#include "vzt/vulkan/instance.hpp"

namespace vzt
{
    inline uint32_t toVersion(uint8_t major, uint8_t minor, uint8_t patch)
    {
        return VK_MAKE_VERSION(major, minor, patch);
    }

    inline VkInstance                      Instance::getHandle() const { return m_handle; }
    inline VulkanVersion                   Instance::getAPIVersion() const { return m_version; }
    inline bool                            Instance::isValidationEnable() const { return m_enableValidation; }
    inline const std::vector<const char*>& Instance::getValidationLayers() const { return m_validationLayers; }
    inline const std::vector<const char*>& Instance::getExtensions() const { return m_extensions; }
} // namespace vzt
