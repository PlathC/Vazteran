#ifndef VZT_SWAPCHAIN_HPP
#define VZT_SWAPCHAIN_HPP

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"
#include "vzt/Device.hpp"

namespace vzt
{
    class Instance;
    class Window;
    class Surface;

    struct SwapchainConfiguration
    {
        uint32_t maxFramesInFlight = 2;
    };

    class Swapchain
    {
      public:
        Swapchain(View<Device> device, View<Surface> surface, Extent2D extent,
                  SwapchainConfiguration configuration = {});
        ~Swapchain();

      private:
        void create();

        void cleanup();

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkSwapchainKHR         m_handle = VK_NULL_HANDLE;
        SwapchainConfiguration m_configuration;

        View<Device>  m_device;
        View<Surface> m_surface;
        Extent2D      m_extent;

        uint32_t                 m_imageNb = 0u;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence>     m_inFlightFences;
        std::vector<VkFence>     m_imagesInFlight;
    };
} // namespace vzt

#endif // VZT_SWAPCHAIN_HPP
