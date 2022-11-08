#include "vzt/Device.hpp"

namespace vzt
{
    inline VkPhysicalDevice PhysicalDevice::getHandle() const { return m_handle; }
    inline VkDevice         Device::getHandle() const { return m_handle; }
    inline PhysicalDevice   Device::getHardware() const { return m_device; }
    inline bool      Device::isSameQueue(const Queue& q1, const Queue& q2) { return q1.getType() < q2.getType(); }
    inline QueueType Queue::getType() const { return m_type; }
    inline uint32_t  Queue::getId() const { return m_id; }
} // namespace vzt
