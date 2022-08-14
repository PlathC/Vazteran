#ifndef VAZTERAN_VULKAN_RENDERER_HPP
#define VAZTERAN_VULKAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/SwapChain.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Window/Window.hpp"

struct GLFWwindow;

namespace vzt
{
	class RenderGraph;
	class RenderPass;
	class Scene;

	class Renderer
	{
	  public:
		using RenderFunction = std::function<void(uint32_t /* imageId */, VkSemaphore /* imageAvailable */,
		                                          VkSemaphore /* renderComplete */, VkFence /* inFlightFence */)>;

		Renderer(Window& window);

		Renderer(const Renderer&)            = delete;
		Renderer& operator=(const Renderer&) = delete;

		Renderer(Renderer&&) noexcept;
		Renderer& operator=(Renderer&&) noexcept;

		~Renderer();

		const Device* getDevice() const { return &m_device; }
		uint32_t      getImageCount() const;

		void setRenderGraph(RenderGraph renderGraph);
		void render();

		void refresh();
		void resize(FrameBufferResize newSize);
		void synchronize();

	  private:
		VkSurfaceKHR m_surface;

		ConnectionHolder m_connectionHolder;

		Device      m_device;
		SwapChain   m_swapChain;
		CommandPool m_commandPool;

		std::optional<RenderGraph> m_renderGraph;
	};
} // namespace vzt

#endif // VAZTERAN_VULKAN_RENDERER_HPP
