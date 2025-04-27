#include "vzt/Vulkan/DeviceObject.hpp"

namespace vzt
{
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
    DeviceObject<Handle>& DeviceObject<Handle>::operator=(DeviceObject<Handle>&& other) noexcept
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
} // namespace vzt