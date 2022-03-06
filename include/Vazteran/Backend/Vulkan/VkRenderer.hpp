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
#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Camera.hpp"

struct GLFWwindow;

namespace vzt
{
	class RenderGraph;
	class GraphicPipeline;
	class ImageView;
	class MeshView;
	class RenderPass;
	class Scene;
	class VkUiRenderer;

	class Renderer
	{
	  public:
		Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size,
		         vzt::RenderGraph renderGraph);

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		Renderer(Renderer&&) noexcept;
		Renderer& operator=(Renderer&&) noexcept;

		~Renderer();

		void               setScene(vzt::Scene* scene);
		void               draw(const vzt::Camera& camera);
		const vzt::Device* getDevice() const { return &m_device; }
		void               resize(vzt::Size2D<uint32_t> newSize);

	  private:
		VkSurfaceKHR m_surface;

		vzt::Device      m_device;
		vzt::SwapChain   m_swapChain;
		vzt::RenderGraph m_renderGraph;

		vzt::Instance* m_instance;
		GLFWwindow*    m_window;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERER_HPP
