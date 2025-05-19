#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    template <class Type>
    std::size_t PhysicalDevice::getUniformAlignment() const
    {
        return getUniformAlignment(sizeof(Type));
    }

    inline VkPhysicalDeviceFeatures   PhysicalDevice::getFeatures() const { return m_features; }
    inline VkPhysicalDeviceProperties PhysicalDevice::getProperties() const { return m_properties; }
    inline VkPhysicalDevice           PhysicalDevice::getHandle() const { return m_handle; }
    inline VkDevice                   Device::getHandle() const { return m_handle; }
    inline const VolkDeviceTable&     Device::getFunctionTable() const { return m_table; }
    inline VmaAllocator               Device::getAllocator() const { return m_allocator; }
    inline PhysicalDevice             Device::getHardware() const { return m_device; }
    inline bool         Device::isSameQueue(const Queue& q1, const Queue& q2) { return q1.getType() < q2.getType(); }
    inline View<Device> Queue::getDevice() const { return m_device; }
    inline VkQueue      Queue::getHandle() const { return m_handle; }
    inline QueueType    Queue::getType() const { return m_type; }
    inline uint32_t     Queue::getId() const { return m_id; }
    inline bool         Queue::canPresent() const { return m_canPresent; }

} // namespace vzt
