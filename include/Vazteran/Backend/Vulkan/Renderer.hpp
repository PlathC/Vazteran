#ifndef VAZTERAN_BACKEND_VULKAN_RENDERER_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/SwapChain.hpp"
#include "Vazteran/Data/Camera.hpp"

struct GLFWwindow;

namespace vzt
{
	class RenderGraph;
	class RenderPass;
	class Scene;
	class Window;

	class Renderer
	{
	  public:
		using RenderFunction = std::function<void(uint32_t /* imageId */, VkSemaphore /* imageAvailable */,
		                                          VkSemaphore /* renderComplete */, VkFence /* inFlightFence */)>;

		Renderer(const Window* window);

		Renderer(const Renderer&)            = delete;
		Renderer& operator=(const Renderer&) = delete;

		Renderer(Renderer&&) noexcept;
		Renderer& operator=(Renderer&&) noexcept;

		~Renderer();

		const vzt::Device* getDevice() const { return &m_device; }
		uint32_t           getImageCount() const;

		void setRenderFunction(RenderFunction drawFunction);
		void render();

		void configure(vzt::RenderGraph& renderGraph);
		void resize(vzt::Size2D<uint32_t> newSize);
		void synchronize();

	  private:
		VkSurfaceKHR m_surface;

		vzt::Device      m_device;
		vzt::SwapChain   m_swapChain;
		vzt::CommandPool m_commandPool;

		RenderFunction m_drawFunction;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERER_HPP
