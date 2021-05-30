#ifndef VAZTERAN_SWAPCHAIN_HPP
#define VAZTERAN_SWAPCHAIN_HPP

#include <chrono>
#include <iostream>
#include <functional>
#include <unordered_set>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Data/Material.hpp"

#include "Vazteran/Vulkan/Buffer.hpp"
#include "Vazteran/Vulkan/GpuObjects.hpp"
#include "Vazteran/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Vulkan/Shader.hpp"
#include "Vazteran/Vulkan/ImageUtils.hpp"

namespace vzt {
    class GraphicPipeline;
    class LogicalDevice;

    using RenderPassFunction = std::function<void(VkCommandBuffer, uint32_t)>;

    struct FrameComponent {
        VkCommandBuffer commandBuffer;
        VkImage colorImage;
        VkImageView colorImageView;

        vzt::FrameBuffer frameBuffer;
    };

    class SwapChain {
    public:
        SwapChain(vzt::LogicalDevice* logicalDevice, VkSurfaceKHR surface, vzt::Size2D<int> frameBufferSize);

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        SwapChain(SwapChain&& other) noexcept;
        SwapChain& operator=(SwapChain&& other) noexcept;

        bool DrawFrame();
        void Recreate(VkSurfaceKHR surface);
        void FrameBufferResized(vzt::Size2D<int> newSize);
        vzt::Size2D<int> FrameBufferSize() const;
        void RecordCommandBuffers(vzt::RenderPassFunction renderPass);
        GraphicPipeline* Pipeline() { return m_graphicPipelines[0].get(); }
        uint32_t ImageCount() const { return m_imageCount; }

        ~SwapChain();

    private:
        void CreateSwapChain();
        void CreateDepthResources();
        void CreateSynchronizationObjects();

        //void UpdateUniformBuffer(uint32_t currentImage, vzt::Transforms ubo);
        void Cleanup();

        constexpr static int MaxFramesInFlight = 2;

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        vzt::LogicalDevice* m_logicalDevice;
        VkSwapchainKHR m_vkHandle;
        std::vector<std::unique_ptr<vzt::GraphicPipeline>> m_graphicPipelines;

        std::size_t m_currentFrame = 0;
        bool m_framebufferResized = false;

        vzt::Size2D<int> m_frameBufferSize;
        VkSurfaceKHR m_surface;
        uint32_t m_imageCount;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        VkCommandPool m_commandPool;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkFence> m_imagesInFlight;

        std::vector<FrameComponent> m_frames;
        std::unique_ptr<vzt::ImageHandler> m_depthImage;
    };
}

#endif //VAZTERAN_SWAPCHAIN_HPP
