#include "vzt/vulkan/device.hpp"

namespace vzt
{
    template <class Type>
    Type& DeviceFeatures::get()
    {
        const VkStructureType type = vku::GetSType<Type>();
        for (std::vector<std::byte>& other : m_others)
        {
            if (reinterpret_cast<VkBaseOutStructure*>(other.data())->sType == type)
                return *reinterpret_cast<Type*>(other.data());
        }

        Type& structure = *reinterpret_cast<Type*>(m_others.emplace_back(sizeof(Type)).data());
        structure.sType = type;
        return structure;
    }

    inline void DeviceBuilder::set(DeviceFeatures features) { m_features = std::move(features); }
    inline void DeviceBuilder::add(QueueType queueType) { m_queueTypes |= queueType; }
    inline void DeviceBuilder::add(dext::Extension extension)
    {
        if (std::ranges::find(m_extensions, std::string_view(extension)) == m_extensions.end())
            m_extensions.emplace_back(extension);
    }

    inline DeviceBuilder& DeviceBuilder::require(Capability capability)
    {
        m_required.emplace_back(std::move(capability));
        return *this;
    }

    inline DeviceBuilder& DeviceBuilder::request(Capability capability)
    {
        m_requested.emplace_back(std::move(capability));
        return *this;
    }

    inline const DeviceFeatures&               DeviceBuilder::getDeviceFeatures() const { return m_features; }
    inline DeviceFeatures&                     DeviceBuilder::getDeviceFeatures() { return m_features; }
    inline QueueType                           DeviceBuilder::getQueueTypes() const { return m_queueTypes; }
    inline const std::vector<dext::Extension>& DeviceBuilder::getExtensions() const { return m_extensions; }
    inline const std::vector<Capability>&      DeviceBuilder::getRequiredCapabilities() const { return m_required; }
    inline const std::vector<Capability>&      DeviceBuilder::getRequestedCapabilities() const { return m_requested; }

    template <class Type>
    std::size_t PhysicalDevice::getUniformAlignment() const
    {
        return getUniformAlignment(sizeof(Type));
    }
    inline VkPhysicalDeviceProperties PhysicalDevice::getProperties() const { return m_properties; }
    inline VkPhysicalDevice           PhysicalDevice::getHandle() const { return m_handle; }
    inline VkDevice                   Device::getHandle() const { return m_handle; }
    inline const VolkDeviceTable&     Device::getFunctionTable() const { return m_table; }
    inline VmaAllocator               Device::getAllocator() const { return m_allocator; }
    inline PhysicalDevice             Device::getHardware() const { return m_device; }
    inline bool Device::isSameQueue(const Queue& q1, const Queue& q2) { return q1.getType() < q2.getType(); }

    template <class Handle>
    DeviceObject<Handle>::DeviceObject(View<Device> device, Handle handle) : m_device(device), m_handle(handle)
    {
    }

    template <class Handle>
    DeviceObject<Handle>::DeviceObject(DeviceObject&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
    }

    template <class Handle>
    DeviceObject<Handle>& DeviceObject<Handle>::operator=(DeviceObject&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);

        return *this;
    }

    template <class Handle>
    View<Device> DeviceObject<Handle>::getDevice() const
    {
        return m_device;
    }

    template <class Handle>
    const Handle& DeviceObject<Handle>::getHandle() const
    {
        return m_handle;
    }

    inline View<Device> Queue::getDevice() const { return m_device; }
    inline VkQueue      Queue::getHandle() const { return m_handle; }
    inline QueueType    Queue::getType() const { return m_type; }
    inline uint32_t     Queue::getId() const { return m_id; }
    inline bool         Queue::canPresent() const { return m_canPresent; }
} // namespace vzt
