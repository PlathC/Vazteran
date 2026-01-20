#ifndef VZT_VULKAN_SWAPCHAIN_HPP
#define VZT_VULKAN_SWAPCHAIN_HPP

#include "vzt/core/math.hpp"
#include "vzt/core/type.hpp"
#include "vzt/vulkan/image.hpp"
#include "vzt/vulkan/type.hpp"

namespace vzt
{
    class Device;
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

    class Swapchain : public DeviceObject<VkSwapchainKHR>
    {
      public:
        Swapchain() = default;
        Swapchain(View<Device> device, View<Surface> surface, SwapchainBuilder configuration = {});

        Swapchain(const Swapchain&)            = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        Swapchain(Swapchain&&) noexcept;
        Swapchain& operator=(Swapchain&&) noexcept;

        ~Swapchain() override;

        // Empty submission if frame buffer changed
        Optional<SwapchainSubmission> getSubmission();
        bool                          present();

        inline void              recreate();
        inline Extent2D          getExtent() const;
        inline View<DeviceImage> getImage(std::size_t i) const;
        inline uint32_t          getImageNb() const;
        inline Format            getFormat() const;

      private:
        void create();
        void cleanup();

        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        SwapchainBuilder m_configuration;

        View<Surface> m_surface            = {};
        Extent2D      m_extent             = {};
        bool          m_framebufferResized = false;
        Format        m_format             = {};

        uint32_t                 m_currentFrame = 0u;
        uint32_t                 m_currentImage = 0u;
        uint32_t                 m_imageNb      = 0u;
        std::vector<VkImage>     m_images{};
        std::vector<DeviceImage> m_userImages{};
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence>     m_inFlightFences;
        std::vector<VkFence>     m_imagesInFlight;
    };
} // namespace vzt

#include "vzt/vulkan/swapchain.inl"

#endif // VZT_VULKAN_SWAPCHAIN_HPP
