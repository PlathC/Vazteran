#ifndef VZT_VULKAN_SWAPCHAIN_HPP
#define VZT_VULKAN_SWAPCHAIN_HPP

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Format.hpp"

namespace vzt
{
    class Device;
    class Image;
    class Instance;
    class Window;
    class Surface;

    struct SwapchainBuilder
    {
        uint32_t maxFramesInFlight = 2;
    };

    struct SwapchainSubmission
    {
        uint32_t    imageId;
        VkSemaphore imageAvailable;
        VkSemaphore renderComplete;
        VkFence     frameComplete;
    };

    class Swapchain
    {
      public:
        Swapchain(View<Device> device, View<Surface> surface, Extent2D extent, SwapchainBuilder configuration = {});

        Swapchain(const Swapchain&)            = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        Swapchain(Swapchain&&) noexcept;
        Swapchain& operator=(Swapchain&&) noexcept;

        ~Swapchain();

        // Empty submission if frame buffer changed
        Optional<SwapchainSubmission> getSubmission();
        bool                          present();

        inline void         setExtent(Extent2D extent);
        inline Extent2D     getExtent() const;
        inline View<Image>  getImage(std::size_t i) const;
        inline uint32_t     getImageNb() const;
        inline Format       getFormat() const;
        inline View<Device> getDevice() const;

      private:
        void create();
        void cleanup();

        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        VkSwapchainKHR   m_handle = VK_NULL_HANDLE;
        SwapchainBuilder m_configuration;

        View<Device>  m_device;
        View<Surface> m_surface;
        Extent2D      m_extent;
        bool          m_framebufferResized = false;
        Format        m_format;

        uint32_t                 m_currentFrame = 0u;
        uint32_t                 m_currentImage = 0u;
        uint32_t                 m_imageNb      = 0u;
        std::vector<VkImage>     m_images{};
        std::vector<Image>       m_userImages{};
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence>     m_inFlightFences;
        std::vector<VkFence>     m_imagesInFlight;
    };
} // namespace vzt

#include "vzt/Vulkan/Swapchain.inl"

#endif // VZT_VULKAN_SWAPCHAIN_HPP
