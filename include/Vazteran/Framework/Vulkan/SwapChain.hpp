#ifndef VAZTERAN_FRAMEWORK_VULKAN_SWAPCHAIN_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_SWAPCHAIN_HPP

#include <chrono>
#include <iostream>
#include <unordered_set>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vazteran/Data/Material.hpp"

#include "Vazteran/Framework/Vulkan/Buffer.hpp"
#include "Vazteran/Framework/Vulkan/CommandPool.hpp"
#include "Vazteran/Framework/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Framework/Vulkan/GpuObjects.hpp"
#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Framework/Vulkan/ImageUtils.hpp"
#include "Vazteran/Framework/Vulkan/RenderPass.hpp"
#include "Vazteran/Framework/Vulkan/Shader.hpp"

namespace vzt
{
	class GraphicPipeline;
	class Device;

	using RenderFunction = std::function<void(uint32_t /* id of current rendered image */,
	                                          VkCommandBuffer /* corresponding command buffer */,
	                                          const vzt::RenderPass* const /* current Render pass*/)>;

	class SwapChain
	{
	  public:
		SwapChain(vzt::Device* device, VkSurfaceKHR surface, vzt::Size2D<uint32_t> swapChainSize);

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		SwapChain(SwapChain&& other) noexcept = default;
		SwapChain& operator=(SwapChain&& other) noexcept = default;

		// Configuration
		void SetFrameBufferSize(vzt::Size2D<uint32_t> newSize);

		void Record(RenderFunction renderFunction);
		void Recreate(VkSurfaceKHR surface);
		bool DrawFrame();

		vzt::PipelineSettings GetSettings() const { return {m_swapChainImageFormat, m_swapChainSize}; }
		vzt::Size2D<uint32_t> FrameBufferSize() const { return m_frameBufferSize; }
		uint32_t              ImageCount() const { return m_imageCount; }

		~SwapChain();

	  private:
		void CreateSwapChain();
		void CreateDepthResources();
		void CreateRenderSupport();
		void CreateSynchronizationObjects();

		void Cleanup();

		constexpr static uint32_t MaxFramesInFlight = 2;

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR          ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D                ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		vzt::Device*   m_device;
		VkSwapchainKHR m_vkHandle;

		std::size_t m_currentFrame       = 0;
		bool        m_framebufferResized = false;

		vzt::Size2D<uint32_t> m_frameBufferSize;
		VkSurfaceKHR          m_surface;
		uint32_t              m_imageCount;
		VkFormat              m_swapChainImageFormat;
		vzt::Size2D<uint32_t> m_swapChainSize;

		std::unique_ptr<vzt::CommandPool> m_commandPool;
		std::vector<VkSemaphore>          m_imageAvailableSemaphores;
		std::vector<VkSemaphore>          m_renderFinishedSemaphores;
		std::vector<VkFence>              m_inFlightFences;
		std::vector<VkFence>              m_imagesInFlight;

		RenderFunction                   m_renderFunction;
		std::unique_ptr<vzt::RenderPass> m_renderPass;
		std::vector<vzt::FrameBuffer>    m_frames;

		std::optional<std::pair<vzt::ImageView, vzt::Sampler>> m_depthImageData;
	};
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_SWAPCHAIN_HPP
