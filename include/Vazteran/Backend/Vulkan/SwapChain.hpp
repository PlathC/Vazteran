#ifndef VAZTERAN_VULKAN_SWAPCHAIN_HPP
#define VAZTERAN_VULKAN_SWAPCHAIN_HPP

#include <chrono>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "Vazteran/Data/Material.hpp"
#include "Vazteran/Math/Math.hpp"

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Backend/Vulkan/Shader.hpp"

namespace vzt
{
	class GraphicPipeline;
	class Device;

	using SubmitFunction = std::function<void(uint32_t /* imageId */, VkSemaphore /* imageAvailable */,
	                                          VkSemaphore /* renderComplete */, VkFence /*inFlightFence*/)>;

	class SwapChain
	{
	  public:
		SwapChain(vzt::Device* device, VkSurfaceKHR surface, Uvec2 swapChainSize);

		SwapChain(const SwapChain&)            = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		SwapChain(SwapChain&& other) noexcept            = default;
		SwapChain& operator=(SwapChain&& other) noexcept = default;

		~SwapChain();

		void                 setRenderPassTemplate(const vzt::RenderPass* const renderPassTemplate);
		std::vector<VkImage> getRenderImages();

		void recreate(VkSurfaceKHR surface);
		bool render(const SubmitFunction submitFunction);

		void        setFrameBufferSize(Uvec2 newSize);
		Uvec2       getFrameBufferSize() const { return m_swapChainSize; }
		uint32_t    getImageCount() const { return m_imageCount; }
		vzt::Format getImageFormat() const { return m_swapChainImageFormat; }

	  private:
		void createSwapChain();
		void createSynchronizationObjects();
		void cleanup();

		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR          chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D                chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	  private:
		constexpr static uint32_t MaxFramesInFlight = 2;

		Device*        m_device;
		VkSwapchainKHR m_vkHandle;

		std::size_t m_currentFrame       = 0;
		bool        m_framebufferResized = false;

		VkSurfaceKHR m_surface;
		uint32_t     m_imageCount;
		Format       m_swapChainImageFormat;
		Uvec2        m_swapChainSize;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence>     m_inFlightFences;
		std::vector<VkFence>     m_imagesInFlight;
	};
} // namespace vzt

#endif // VAZTERAN_BVULKAN_SWAPCHAIN_HPP
