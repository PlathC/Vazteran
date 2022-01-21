#include "Vazteran/Backend/Vulkan/VkRenderer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/VkUiRenderer.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"

namespace vzt
{
	Renderer::Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size)
	    : m_surface(surface), m_device(instance, m_surface), m_swapChain(&m_device, surface, size),
	      m_commandPool(&m_device)
	{
		m_renderPass = std::make_unique<vzt::RenderPass>(&m_device, m_swapChain.GetImageFormat());
		m_swapChain.SetRenderPassTemplate(m_renderPass.get());

		const uint32_t imageCount = m_swapChain.ImageCount();
		m_commandPool.AllocateCommandBuffers(imageCount);

		m_meshView = std::make_unique<vzt::MeshView>(&m_device, m_swapChain.ImageCount());
		m_meshView->Configure(m_swapChain.GetSettings());

		vzt::ui::UiManager     uiManager;
		vzt::ui::MainMenuBar   mainMenuBar;
		vzt::ui::MainMenuField fileMenuField = vzt::ui::MainMenuField("File");
		fileMenuField.AddItem(vzt::ui::MainMenuItem("New", []() { std::cout << "New" << std::endl; }));
		fileMenuField.AddItem(vzt::ui::MainMenuItem("Open", []() { std::cout << "Open" << std::endl; }));
		mainMenuBar.AddMenu(std::move(fileMenuField));

		vzt::ui::MainMenuField brdfMenuField = vzt::ui::MainMenuField("BRDF");
		brdfMenuField.AddItem(vzt::ui::MainMenuItem("Blinn-Phong", []() { std::cout << "Blinn-Phong" << std::endl; }));
		mainMenuBar.AddMenu(std::move(brdfMenuField));

		uiManager.SetMainMenuBar(mainMenuBar);

		m_ui = std::make_unique<vzt::VkUiRenderer>(instance, &m_device, window, m_swapChain.ImageCount(),
		                                           m_renderPass.get(), std::move(uiManager));
	}

	Renderer::~Renderer() = default;

	void Renderer::SetScene(vzt::Scene* scene)
	{
		for (const auto* model : scene->Models())
		{
			m_meshView->AddModel(model);
		}
	}

	void Renderer::Draw(const vzt::Camera& camera)
	{
		m_meshView->Update(camera);
		if (m_swapChain.DrawFrame([this](uint32_t imageId, const vzt::FrameBuffer* const frameBuffer) {
			    return Record(imageId, frameBuffer);
		    }))
		{
			vkDeviceWaitIdle(m_device.VkHandle());
		}
	}

	void Renderer::FrameBufferResized(vzt::Size2D<uint32_t> newSize)
	{
		m_swapChain.SetFrameBufferSize(std::move(newSize));
		m_swapChain.Recreate(m_surface);
		m_meshView->Configure(m_swapChain.GetSettings());
	}

	std::vector<VkCommandBuffer> Renderer::Record(uint32_t imageId, const vzt::FrameBuffer* const frameBuffer)
	{
		m_commandPool.RecordBuffer(imageId, [&](VkCommandBuffer commandBuffer) {
			m_renderPass->Bind(commandBuffer, frameBuffer);
			m_meshView->Record(imageId, commandBuffer, m_renderPass.get());
			m_ui->Record(imageId, commandBuffer, m_renderPass.get());
			m_renderPass->Unbind(commandBuffer);
		});

		return {(m_commandPool[imageId])};
	} // namespace vzt

} // namespace vzt
