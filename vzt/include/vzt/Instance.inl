#include "vzt/Instance.hpp"

namespace vzt
{
    inline VkInstance                   Instance::getHandle() const { return m_handle; }
    inline const InstanceConfiguration& Instance::getConfiguration() const { return m_configuration; }
} // namespace vzt