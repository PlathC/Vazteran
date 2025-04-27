#ifndef VZT_VULKAN_DEVICE_OBJECT_HPP
#define VZT_VULKAN_DEVICE_OBJECT_HPP

#include <volk.h>

#include "vzt/Core/Type.hpp"

#ifndef VK_DEFINE_NON_DISPATCHABLE_HANDLE
#if (VK_USE_64_BIT_PTR_DEFINES == 1)
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201103L))
#define VK_NULL_HANDLE nullptr
#else
#define VK_NULL_HANDLE ((void*)0)
#endif
#else
#define VK_NULL_HANDLE 0ULL
#endif
#endif
#ifndef VK_NULL_HANDLE
#define VK_NULL_HANDLE 0
#endif

namespace vzt
{
    class Device;

    template <class Handle>
    class DeviceObject
    {
      public:
        DeviceObject() = default;
        DeviceObject(View<Device> device, Handle handle = nullptr);

        DeviceObject(const DeviceObject&)            = delete;
        DeviceObject& operator=(const DeviceObject&) = delete;

        DeviceObject(DeviceObject&& other) noexcept;
        DeviceObject& operator=(DeviceObject&& other) noexcept;

        virtual ~DeviceObject() = default;

        View<Device>  getDevice() const;
        const Handle& getHandle() const;

      protected:
        View<Device> m_device;
        Handle       m_handle = VK_NULL_HANDLE;
    };
} // namespace vzt

#include "vzt/Vulkan/DeviceObject.inl"

#endif // VZT_VULKAN_OBJECT_HPP