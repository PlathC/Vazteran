#ifndef VAZTERAN_BACKEND_VULKAN_SWAPCHAIN_HPP
#define VAZTERAN_BACKEND_VULKAN_SWAPCHAIN_HPP

#include <chrono>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Data/Material.hpp"

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GpuObjects.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Backend/Vulkan/Shader.hpp"

namespace vzt
{
	class GraphicPipeline;
	class Device;

	using RenderFunction = std::function<std::vector<VkCommandBuffer>(
	    uint32_t /* id of current rendered image */, const vzt::FrameBuffer* const /* current framebuffer */)>;

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
		void SetRenderPassTemplate(const vzt::RenderPass* const renderPassTemplate);

		void Recreate(VkSurfaceKHR surface);
		bool DrawFrame(RenderFunction renderFunction);

		vzt::PipelineSettings GetSettings() const
		{
			return {m_renderPassTemplate, m_swapChainImageFormat, m_swapChainSize};
		}
		vzt::Size2D<uint32_t> FrameBufferSize() const { return m_swapChainSize; }
		uint32_t              ImageCount() const { return m_imageCount; }
		VkFormat              GetImageFormat() const { return m_swapChainImageFormat; }

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

		VkSurfaceKHR          m_surface;
		uint32_t              m_imageCount;
		VkFormat              m_swapChainImageFormat;
		vzt::Size2D<uint32_t> m_swapChainSize;

		// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFramebufferCreateInfo.html
		//  what render passes the framebuffer will be compatible with
		const vzt::RenderPass*                                 m_renderPassTemplate = nullptr;
		std::vector<vzt::FrameBuffer>                          m_frames;
		std::optional<std::pair<vzt::ImageView, vzt::Sampler>> m_depthImageData;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence>     m_inFlightFences;
		std::vector<VkFence>     m_imagesInFlight;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_SWAPCHAIN_HPP
