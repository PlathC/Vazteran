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

    struct Submission
    {
        uint32_t    imageId;
        VkSemaphore imageAvailable;
        VkSemaphore renderComplete;
        VkFence     frameComplete;
    };

    class Swapchain
    {
      public:
        Swapchain(View<Device> device, View<Surface> surface, Extent2D extent,
                  SwapchainConfiguration configuration = {});
        ~Swapchain();

        // Empty submission if frame buffer changed
        std::optional<Submission> getSubmission();
        bool                      present(const Submission& submission);

        inline const std::vector<VkImage>& getImages() const;

      private:
        void create();
        void cleanup();

        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        VkSwapchainKHR         m_handle = VK_NULL_HANDLE;
        SwapchainConfiguration m_configuration;

        View<Device>  m_device;
        View<Surface> m_surface;
        Extent2D      m_extent;
        bool          m_framebufferResized = false;

        uint32_t                 m_currentFrame = 0u;
        uint32_t                 m_imageNb      = 0u;
        std::vector<VkImage>     m_images{};
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence>     m_inFlightFences;
        std::vector<VkFence>     m_imagesInFlight;
    };
} // namespace vzt

#include "vzt/Swapchain.inl"

#endif // VZT_SWAPCHAIN_HPP
