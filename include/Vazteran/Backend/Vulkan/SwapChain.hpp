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

	using RenderFunction = std::function<std::vector<VkCommandBuffer>(uint32_t /* id of current rendered image */)>;

	class SwapChain
	{
	  public:
		SwapChain(vzt::Device* device, VkSurfaceKHR surface, vzt::Size2D<uint32_t> swapChainSize);

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		SwapChain(SwapChain&& other) noexcept = default;
		SwapChain& operator=(SwapChain&& other) noexcept = default;

		void SetFrameBufferSize(vzt::Size2D<uint32_t> newSize);
		void SetRenderPassTemplate(const vzt::RenderPass* const renderPassTemplate);

		void Recreate(VkSurfaceKHR surface);
		bool DrawFrame(RenderFunction renderFunction);

		std::vector<VkImage>  GetImagesKHR();
		vzt::Size2D<uint32_t> GetFrameBufferSize() const { return m_swapChainSize; }
		uint32_t              GetImageCount() const { return m_imageCount; }
		vzt::Format           GetImageFormat() const { return m_swapChainImageFormat; }

		~SwapChain();

	  private:
		void CreateSwapChain();
		void CreateSynchronizationObjects();
		void Cleanup();

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR          ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D                ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	  private:
		constexpr static uint32_t MaxFramesInFlight = 2;

		vzt::Device*   m_device;
		VkSwapchainKHR m_vkHandle;

		std::size_t m_currentFrame       = 0;
		bool        m_framebufferResized = false;

		VkSurfaceKHR          m_surface;
		uint32_t              m_imageCount;
		vzt::Format           m_swapChainImageFormat;
		vzt::Size2D<uint32_t> m_swapChainSize;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence>     m_inFlightFences;
		std::vector<VkFence>     m_imagesInFlight;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_SWAPCHAIN_HPP
