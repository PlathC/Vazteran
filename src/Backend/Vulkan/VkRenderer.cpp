#include "Vazteran/Backend/Vulkan/VkRenderer.hpp"
#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/VkUiRenderer.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"

namespace vzt
{
	Renderer::Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size,
	                   vzt::RenderGraph renderGraph)
	    : m_surface(surface), m_device(instance, m_surface), m_swapChain(&m_device, surface, size),
	      m_instance(instance), m_window(window), m_renderGraph(std::move(renderGraph))
	{
		m_renderGraph.configure(&m_device, m_swapChain.getRenderImages(), m_swapChain.getFrameBufferSize(),
		                        m_swapChain.getImageFormat(), m_device.getPhysicalDevice()->findDepthFormat());
	}

	Renderer::Renderer(Renderer&& other) noexcept
	    : m_device(std::move(other.m_device)), m_swapChain(std::move(other.m_swapChain))
	{
		std::swap(m_surface, other.m_surface);
		std::swap(m_renderGraph, other.m_renderGraph);
		std::swap(m_instance, other.m_instance);
		std::swap(m_window, other.m_window);
	}

	Renderer& Renderer::operator=(Renderer&& other) noexcept
	{
		std::swap(m_surface, other.m_surface);
		std::swap(m_renderGraph, other.m_renderGraph);

		std::swap(m_device, other.m_device);
		std::swap(m_swapChain, other.m_swapChain);

		std::swap(m_instance, other.m_instance);
		std::swap(m_window, other.m_window);

		return *this;
	}

	Renderer::~Renderer() = default;

	void Renderer::setScene(vzt::Scene* scene)
	{
		auto uiData = scene->cSceneUi();
		if (uiData.has_value())
		{
			// m_ui = std::make_unique<vzt::VkUiRenderer>(m_instance, &m_device, m_window, m_swapChain.imageCount(),
			//                                            m_frames[0].getRenderPass(), uiData.value());
		}

		// auto sceneModels = scene->cModels();
		// for (const auto* model : sceneModels)
		// {
		// 	m_meshView->addModel(model);
		// }
	}

	void Renderer::draw(const vzt::Camera& camera)
	{
		// m_meshView->update(camera);

		const bool recreate = m_swapChain.render(
		    [&](uint32_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete, VkFence inFlightFence) {
			    m_renderGraph.render(imageId, imageAvailable, renderComplete, inFlightFence);
		    });

		if (recreate)
		{
			vkDeviceWaitIdle(m_device.vkHandle());
		}
	}

	void Renderer::resize(vzt::Size2D<uint32_t> newSize)
	{
		m_swapChain.setFrameBufferSize(std::move(newSize));
		m_swapChain.recreate(m_surface);

		m_renderGraph.configure(&m_device, m_swapChain.getRenderImages(), m_swapChain.getFrameBufferSize(),
		                        m_swapChain.getImageFormat(), m_device.getPhysicalDevice()->findDepthFormat());
	}
} // namespace vzt
