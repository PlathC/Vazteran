#include "vzt/Instance.hpp"

namespace vzt
{
    inline VkInstance                      Instance::getHandle() const { return m_handle; }
    inline bool                            Instance::isValidationEnable() const { return m_enableValidation; }
    inline const std::vector<const char*>& Instance::getValidationLayers() const { return m_validationLayers; }
    inline const std::vector<const char*>& Instance::getExtensions() const { return m_extensions; }
} // namespace vzt
