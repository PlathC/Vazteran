#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/View/UiView.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	UiView::UiView(const Window& window, uint32_t imageCount)
	    : View(imageCount), m_window(&window), m_instance(window.getInstance())
	{
	}

	UiView::UiView(UiView&& other) noexcept : View(static_cast<View&&>(other))
	{
		std::swap(m_instance, other.m_instance);
		std::swap(m_window, other.m_window);
		std::swap(m_manager, other.m_manager);
		std::swap(m_descriptorPool, other.m_descriptorPool);
		std::swap(m_isInitialized, other.m_isInitialized);
	}

	UiView& UiView::operator=(UiView&& other) noexcept
	{
		std::swap(m_instance, other.m_instance);
		std::swap(m_window, other.m_window);
		std::swap(m_manager, other.m_manager);
		std::swap(m_descriptorPool, other.m_descriptorPool);
		std::swap(m_isInitialized, other.m_isInitialized);

		return static_cast<UiView&>(View::operator=(std::move(other)));
	}

	UiView::~UiView()
	{
		if (m_isInitialized)
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
	}

	void UiView::setManager(const ui::UiManager& manager) { m_manager = &manager; }

	void UiView::apply(RenderGraph& graph, AttachmentHandle& depth, AttachmentHandle& composed)
	{
		auto& UiPass = graph.addPass("UiPass", vzt::QueueType::Graphic);
		UiPass.setDepthStencilOutput(depth, "Depth");
		UiPass.addColorInputOutput(composed, "Composed", "ComposedWithUi");

		UiPass.setConfigureFunction([this](const vzt::PipelineContextSettings& settings) { configure(settings); });

		UiPass.setRecordFunction(
		    [this](uint32_t imageId, VkCommandBuffer cmd, const std::vector<VkDescriptorSet>& engineDescriptorSets) {
			    record(imageId, cmd, engineDescriptorSets);
		    });
	}

	void UiView::refresh() {}

	void UiView::configure(const PipelineContextSettings& settings)
	{
		if (m_isInitialized)
			return;

		const std::vector<vzt::DescriptorType> descriptorTypes = {vzt::DescriptorType::Sampler,
		                                                          vzt::DescriptorType::CombinedSampler,
		                                                          vzt::DescriptorType::SampledImage,
		                                                          vzt::DescriptorType::StorageImage,
		                                                          vzt::DescriptorType::UniformTexelBuffer,
		                                                          vzt::DescriptorType::StorageTexelBuffer,
		                                                          vzt::DescriptorType::UniformBuffer,
		                                                          vzt::DescriptorType::StorageBuffer,
		                                                          vzt::DescriptorType::UniformBufferDynamic,
		                                                          vzt::DescriptorType::StorageBufferDynamic,
		                                                          vzt::DescriptorType::InputAttachment};
		m_descriptorPool = vzt::DescriptorPool(settings.device, descriptorTypes, 1000,
		                                       VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(m_window->getWindowHandle(), true);

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance                  = m_instance->vkHandle();
		initInfo.PhysicalDevice            = settings.device->getPhysicalDevice()->vkHandle();
		initInfo.Device                    = settings.device->vkHandle();

		QueueFamilyIndices indices = settings.device->getDeviceQueueFamilyIndices();
		initInfo.QueueFamily       = indices.graphicsFamily.value();
		initInfo.Queue             = settings.device->getGraphicsQueue();
		// initInfo.PipelineCache             = g_PipelineCache;
		initInfo.DescriptorPool = m_descriptorPool.vkHandle();
		initInfo.Subpass        = 0;
		initInfo.MinImageCount  = m_imageNb;
		initInfo.ImageCount     = m_imageNb;
		initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
		// initInfo.Allocator                 = nullptr;
		// initInfo.CheckVkResultFn           = nullptr;
		ImGui_ImplVulkan_Init(&initInfo, settings.renderPassTemplate->vkHandle());

		settings.device->singleTimeCommand(
		    [](VkCommandBuffer commandBuffer) { ImGui_ImplVulkan_CreateFontsTexture(commandBuffer); });

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		ImGui_ImplVulkan_SetMinImageCount(m_imageNb);

		m_isInitialized = true;
	}

	void UiView::record(uint32_t imageId, VkCommandBuffer cmd,
	                    const std::vector<VkDescriptorSet>& engineDescriptorSets) const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (m_manager)
			m_manager->draw();

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		ImGui_ImplVulkan_RenderDrawData(drawData, cmd);
	}
} // namespace vzt
