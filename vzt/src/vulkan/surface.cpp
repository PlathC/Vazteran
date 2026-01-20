#include "vzt/vulkan/surface.hpp"

#include <SDL3/SDL_vulkan.h>

#include "vzt/core/logger.hpp"
#include "vzt/window.hpp"

namespace vzt
{
    Surface::Surface(const Window& window, const Instance& instance) : m_window(window), m_instance(&instance)
    {
        if (!SDL_Vulkan_CreateSurface(m_window->getHandle(), m_instance->getHandle(), nullptr, &m_handle))
            logger::error("[SDL] {}", SDL_GetError());
    }

    Surface::Surface(Surface&& other) noexcept
    {
        std::swap(m_window, other.m_window);
        std::swap(m_instance, other.m_instance);
        std::swap(m_handle, other.m_handle);
    }

    Surface& Surface::operator=(Surface&& other) noexcept
    {
        std::swap(m_window, other.m_window);
        std::swap(m_instance, other.m_instance);
        std::swap(m_handle, other.m_handle);

        return *this;
    }

    Surface::~Surface()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_instance->getHandle(), m_handle, nullptr);
    }

    VkSurfaceCapabilitiesKHR Surface::getCapabilities(View<Device> device) const
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->getHardware().getHandle(), m_handle, &capabilities);
        return capabilities;
    }

    std::vector<VkSurfaceFormatKHR> Surface::getFormats(View<Device> device) const
    {
        const VkPhysicalDevice physicalDevice = device->getHardware().getHandle();

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_handle, &formatCount, nullptr);

        std::vector<VkSurfaceFormatKHR> formats{};
        if (formatCount != 0)
        {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_handle, &formatCount, formats.data());
        }

        return formats;
    }

    std::vector<VkPresentModeKHR> Surface::getPresentModes(View<Device> device) const
    {
        const VkPhysicalDevice physicalDevice = device->getHardware().getHandle();

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_handle, &presentModeCount, nullptr);

        std::vector<VkPresentModeKHR> presentModes{};
        if (presentModeCount != 0)
        {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_handle, &presentModeCount, presentModes.data());
        }

        return presentModes;
    }

    Extent2D Surface::getExtent() const
    {
        int width, height;
        SDL_GetWindowSizeInPixels(m_window->getHandle(), &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }
} // namespace vzt
