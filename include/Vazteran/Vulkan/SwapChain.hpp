#ifndef VAZTERAN_SWAPCHAIN_HPP
#define VAZTERAN_SWAPCHAIN_HPP

#include <chrono>
#include <iostream>
#include <functional>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Vazteran/Vulkan/GpuObject.hpp"
#include "Vazteran/Vulkan/Texture.hpp"
#include "Vazteran/Vulkan/FrameBuffer.hpp"

namespace vzt {
    class GraphicPipeline;
    class LogicalDevice;

    using RenderPassFunction = std::function<void(VkCommandBuffer, VkDescriptorSet&, GraphicPipeline*)>;

    class SwapChain {
    public:
        SwapChain(LogicalDevice* logicalDevice, VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight,
                  RenderPassFunction renderPass, std::unique_ptr<TextureImage> textureImage);

        bool DrawFrame(UniformBufferObject ubo);
        void Recreate(VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight);
        void FrameBufferResized() { m_framebufferResized = true; };
        uint32_t Width() const { return m_frameBufferWidth; }
        uint32_t Height() const { return m_frameBufferHeight; }

        ~SwapChain();

    private:
        void CreateSwapChain();
        void CreateImageKHR();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateFrameBuffers();
        void CreateCommandBuffers();
        void CreateSynchronizationObjects();

        void UpdateUniformBuffer(uint32_t currentImage, UniformBufferObject ubo);
        void UpdateDescriptorSets();
        void Cleanup();

        constexpr static int MaxFramesInFlight = 2;

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        LogicalDevice* m_logicalDevice;
        VkSwapchainKHR m_vkHandle;
        std::unique_ptr<GraphicPipeline> m_graphicPipeline;

        std::size_t m_currentFrame = 0;
        bool m_framebufferResized = false;

        int m_frameBufferWidth;
        int m_frameBufferHeight;
        RenderPassFunction m_renderPass;
        VkSurfaceKHR m_surface;
        uint32_t m_imageCount;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        VkCommandPool m_commandPool;
        VkDescriptorPool m_descriptorPool;

        std::vector<VkDescriptorSet> m_descriptorSets;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkFence> m_imagesInFlight;

        VkImage m_depthImage;
        VkDeviceMemory m_depthImageMemory;
        VkImageView m_depthImageView;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<std::unique_ptr<FrameBuffer>> m_frameBuffers;

        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;

        std::unique_ptr<TextureSampler> m_textureSampler;
        std::unique_ptr<TextureImage> m_textureImage;
    };
}

#endif //VAZTERAN_SWAPCHAIN_HPP
