#include "Vazteran/Backend/Vulkan/VkRenderer.hpp"
#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/VkUiRenderer.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"

namespace vzt
{
	Renderer::Renderer(vzt::Instance* instance, GLFWwindow* window, VkSurfaceKHR surface, vzt::Size2D<uint32_t> size)
	    : m_surface(surface), m_device(instance, m_surface), m_swapChain(&m_device, surface, size),
	      m_commandPool(&m_device), m_instance(instance), m_window(window)
	{
		const uint32_t imageCount           = m_swapChain.GetImageCount();
		const auto     swapChainImageFormat = m_swapChain.GetImageFormat();
		const auto     swapChainSize        = m_swapChain.GetFrameBufferSize();

		m_renderPass            = std::make_unique<vzt::RenderPass>(&m_device, swapChainImageFormat);
		vzt::Format depthFormat = m_device.ChosenPhysicalDevice()->FindDepthFormat();

		m_depthAttachment = std::make_unique<vzt::Attachment>(&m_device, swapChainSize, depthFormat,
		                                                      vzt::ImageUsage::DepthStencilAttachment);

		auto swapChainImages = m_swapChain.GetImagesKHR();
		m_frames.reserve(imageCount);
		for (std::size_t i = 0; i < imageCount; i++)
		{
			m_colorAttachments.emplace_back(&m_device, swapChainImages[i], swapChainImageFormat,
			                                vzt::ImageAspect::Color);

			std::vector<const vzt::Attachment*> currentAttachments = {
			    &m_colorAttachments[m_colorAttachments.size() - 1], m_depthAttachment.get()};

			m_frames.emplace_back(&m_device, m_renderPass.get(), currentAttachments, swapChainSize);
		}

		m_commandPool.AllocateCommandBuffers(imageCount);

		m_meshView = std::make_unique<vzt::MeshView>(&m_device, imageCount);
		m_meshView->Configure({m_renderPass.get(), swapChainImageFormat, swapChainSize});
	}

	Renderer::~Renderer() = default;

	void Renderer::SetScene(vzt::Scene* scene)
	{
		auto uiData = scene->CSceneUi();
		if (uiData.has_value())
		{
			m_ui = std::make_unique<vzt::VkUiRenderer>(m_instance, &m_device, m_window, m_swapChain.GetImageCount(),
			                                           m_renderPass.get(), uiData.value());
		}

		auto sceneModels = scene->CModels();
		for (const auto* model : sceneModels)
		{
			m_meshView->AddModel(model);
		}
	}

	void Renderer::Draw(const vzt::Camera& camera)
	{
		m_meshView->Update(camera);
		if (m_swapChain.DrawFrame([this](uint32_t imageId) { return Record(imageId); }))
		{
			vkDeviceWaitIdle(m_device.VkHandle());
		}
	}

	void Renderer::FrameBufferResized(vzt::Size2D<uint32_t> newSize)
	{
		m_swapChain.SetFrameBufferSize(std::move(newSize));
		m_swapChain.Recreate(m_surface);

		m_colorAttachments.clear();
		m_frames.clear();

		const uint32_t imageCount           = m_swapChain.GetImageCount();
		const auto     swapChainImageFormat = m_swapChain.GetImageFormat();
		const auto     swapChainSize        = m_swapChain.GetFrameBufferSize();

		vzt::Format depthFormat = m_device.ChosenPhysicalDevice()->FindDepthFormat();

		m_depthAttachment = std::make_unique<vzt::Attachment>(&m_device, swapChainSize, depthFormat,
		                                                      vzt::ImageUsage::DepthStencilAttachment);

		auto swapChainImages = m_swapChain.GetImagesKHR();
		for (std::size_t i = 0; i < imageCount; i++)
		{
			m_colorAttachments.emplace_back(&m_device, swapChainImages[i], swapChainImageFormat,
			                                vzt::ImageAspect::Color);

			std::vector<const vzt::Attachment*> currentAttachments = {
			    &m_colorAttachments[m_colorAttachments.size() - 1], m_depthAttachment.get()};

			m_frames.emplace_back(&m_device, m_renderPass.get(), currentAttachments, swapChainSize);
		}

		m_meshView->Configure({m_renderPass.get(), swapChainImageFormat, swapChainSize});
	} // namespace vzt

	std::vector<VkCommandBuffer> Renderer::Record(uint32_t imageId)
	{
		m_commandPool.RecordBuffer(imageId, [&](VkCommandBuffer commandBuffer) {
			m_renderPass->Bind(commandBuffer, &m_frames[imageId]);
			m_meshView->Record(imageId, commandBuffer, m_renderPass.get());

			if (m_ui)
			{
				m_ui->Record(imageId, commandBuffer, m_renderPass.get());
			}
			m_renderPass->Unbind(commandBuffer);
		});

		return {(m_commandPool[imageId])};
	} // namespace vzt

} // namespace vzt
