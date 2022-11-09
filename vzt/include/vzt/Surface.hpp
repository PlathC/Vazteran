#ifndef VZT_SURFACE_HPP
#define VZT_SURFACE_HPP

#include <vector>

#include "vzt/Window.hpp"

namespace vzt
{
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

#include "vzt/Surface.inl"

#endif // VZT_SURFACE_HPP
