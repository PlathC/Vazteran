#ifndef VZT_VULKAN_SURFACE_HPP
#define VZT_VULKAN_SURFACE_HPP

#include <vector>

#include <vulkan/vulkan_core.h>

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Type.hpp"

namespace vzt
{
    class Device;
    class Instance;
    class Window;

    class Surface
    {
      public:
        Surface(const Window& window, const Instance& instance);

        Surface(const Surface&)            = delete;
        Surface& operator=(const Surface&) = delete;

        Surface(Surface&&) noexcept;
        Surface& operator=(Surface&&) noexcept;

        ~Surface();

        VkSurfaceCapabilitiesKHR        getCapabilities(View<Device> device) const;
        std::vector<VkSurfaceFormatKHR> getFormats(View<Device> device) const;
        std::vector<VkPresentModeKHR>   getPresentModes(View<Device> device) const;
        Extent2D                        getExtent() const;

        inline VkSurfaceKHR getHandle() const;

      private:
        View<Window>   m_window;
        View<Instance> m_instance;

        VkSurfaceKHR m_handle = VK_NULL_HANDLE;
    };
} // namespace vzt

#include "vzt/Vulkan/Surface.inl"

#endif // VZT_VULKAN_SURFACE_HPP
