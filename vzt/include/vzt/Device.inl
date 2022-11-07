#include "vzt/Device.hpp"

namespace vzt
{
    inline VkPhysicalDevice PhysicalDevice::getHandle() const { return m_handle; }
    inline VkDevice         Device::getHandle() const { return m_handle; }
    inline QueueType        Queue::getType() const { return m_type; }
} // namespace vzt
