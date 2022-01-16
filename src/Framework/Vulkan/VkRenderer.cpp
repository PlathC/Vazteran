#include "Vazteran/Framework/Vulkan/VkRenderer.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Framework/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Framework/Vulkan/MeshView.hpp"

namespace vzt
{
	Renderer::Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size)
	    : m_surface(surface), m_device(std::make_unique<vzt::Device>(instance, m_surface))
	{
		m_swapChain = std::make_unique<vzt::SwapChain>(m_device.get(), surface, size);

		m_meshView = std::make_unique<vzt::MeshView>(m_device.get(), m_swapChain->ImageCount());
		m_meshView->Configure(m_swapChain->GetSettings());
	}

	Renderer::~Renderer() = default;

	void Renderer::SetScene(vzt::Scene* scene)
	{
		for (const auto* model : scene->Models())
		{
			m_meshView->AddModel(model);
		}

		m_swapChain->Record([&](uint32_t i, VkCommandBuffer commandBuffer, const vzt::RenderPass* const renderPass) {
			m_meshView->Record(i, commandBuffer, renderPass);
		});
	}

	void Renderer::Draw(const vzt::Camera& camera)
	{
		m_meshView->Update(camera);
		if (m_swapChain->DrawFrame())
		{
			vkDeviceWaitIdle(m_device->VkHandle());
		}
	}

	void Renderer::FrameBufferResized(vzt::Size2D<uint32_t> newSize)
	{
		m_swapChain->SetFrameBufferSize(std::move(newSize));
		m_swapChain->Recreate(m_surface);
		m_meshView->Configure(m_swapChain->GetSettings());
		m_swapChain->Record([&](uint32_t i, VkCommandBuffer commandBuffer, const vzt::RenderPass* const renderPass) {
			m_meshView->Record(i, commandBuffer, renderPass);
		});
	}
} // namespace vzt
