#ifndef VAZTERAN_SWAPCHAIN_HPP
#define VAZTERAN_SWAPCHAIN_HPP

#include <iostream>
#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

#include "Vazteran/Vulkan/FrameBuffer.hpp"

namespace vzt {
    class GraphicPipeline;
    class LogicalDevice;

    using RenderPassFunction = std::function<void(VkCommandBuffer)>;

    class SwapChain {
    public:
        SwapChain(LogicalDevice* logicalDevice, VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight,
                  RenderPassFunction renderPass);

        bool DrawFrame();
        void Recreate(VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight);
        void FrameBufferResized() { m_framebufferResized = true; };

        ~SwapChain();

    private:
        void CreateSwapChain();
        void CreateImageKHR();
        void CreateImageViews();
        void CreateCommandBuffers();
        void CreateSynchronizationObjects();

        constexpr static int MaxFramesInFlight = 2;

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        int m_frameBufferWidth;
        int m_frameBufferHeight;
        RenderPassFunction m_renderPass;
        VkSurfaceKHR m_surface;
        uint32_t m_imageCount;
        LogicalDevice* m_logicalDevice;
        VkSwapchainKHR m_swapChain;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkFence> m_imagesInFlight;

        std::size_t m_currentFrame = 0;
        bool m_framebufferResized = false;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::unique_ptr<GraphicPipeline> m_graphicPipeline;
        std::vector<std::unique_ptr<FrameBuffer>> m_frameBuffers;
    };
}


#endif //VAZTERAN_SWAPCHAIN_HPP
