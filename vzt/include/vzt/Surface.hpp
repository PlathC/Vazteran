#ifndef VZT_SURFACE_HPP
#define VZT_SURFACE_HPP

#include <unordered_map>
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
        inline VkSurfaceKHR             getHandle() const;

      private:
        const Instance*                        m_instance = nullptr;
        VkSurfaceKHR                           m_handle   = VK_NULL_HANDLE;
        std::unordered_map<QueueType, VkQueue> m_queues;
    };
} // namespace vzt

#include "vzt/Surface.inl"

#endif // VZT_SURFACE_HPP
