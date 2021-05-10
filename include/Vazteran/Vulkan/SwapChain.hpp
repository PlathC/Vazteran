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
#include "Vazteran/Vulkan/GpuObject.hpp"
#include "Vazteran/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Vulkan/Shader.hpp"
#include "Vazteran/Vulkan/Texture.hpp"

namespace vzt {
    class GraphicPipeline;
    class LogicalDevice;

    using RenderPassFunction = std::function<void(VkCommandBuffer, VkDescriptorSet&, GraphicPipeline*)>;

    struct RenderComponent {

    };

    class SwapChain {
    public:
        SwapChain(vzt::LogicalDevice* logicalDevice, VkSurfaceKHR surface, int frameBufferWidth, int frameBufferHeight,
                  vzt::RenderPassFunction renderPass, std::unordered_set<vzt::Shader, vzt::ShaderHash> shaders);

        SwapChain(SwapChain&) = delete;
        SwapChain& operator=(SwapChain&) = delete;

        SwapChain(SwapChain&& other) noexcept;
        SwapChain& operator=(SwapChain&& other) noexcept;

        bool DrawFrame(vzt::Transforms ubo);
        void Recreate(VkSurfaceKHR surface);
        void FrameBufferResized(vzt::Size2D newSize);
        vzt::Size2D FrameBufferSize() const;

        ~SwapChain();

    private:
        void CreateSwapChain();
        void CreateImageKHR();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateFrameBuffers();
        void CreateCommandBuffers();
        void CreateSynchronizationObjects();

        void UpdateUniformBuffer(uint32_t currentImage, vzt::Transforms ubo);
        void UpdateDescriptorSets();
        void Cleanup();

        constexpr static int MaxFramesInFlight = 2;

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        vzt::LogicalDevice* m_logicalDevice;
        VkSwapchainKHR m_vkHandle;
        std::unique_ptr<vzt::GraphicPipeline> m_graphicPipeline;

        std::size_t m_currentFrame = 0;
        bool m_framebufferResized = false;

        int m_frameBufferWidth;
        int m_frameBufferHeight;
        vzt::RenderPassFunction m_renderPass;
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
        std::vector<std::unique_ptr<vzt::FrameBuffer>> m_frameBuffers;

        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;

        // TODO: Get this out from the swapchain
        std::unordered_set<vzt::Shader, vzt::ShaderHash> m_shaders;
    };
}

#endif //VAZTERAN_SWAPCHAIN_HPP
