#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/Instance.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Backend/Vulkan/UiRenderer.hpp"

namespace vzt
{
	UiRenderer::UiRenderer(UiRenderer&& other) noexcept
	{
		std::swap(m_descriptorPool, other.m_descriptorPool);
		std::swap(m_isInitialized, other.m_isInitialized);
	}
	UiRenderer& UiRenderer::operator=(UiRenderer&& other) noexcept
	{
		std::swap(m_descriptorPool, other.m_descriptorPool);
		std::swap(m_isInitialized, other.m_isInitialized);

		return *this;
	}

	UiRenderer::~UiRenderer()
	{
		if (m_isInitialized)
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
	}

	void UiRenderer::configure(const Instance* const instance, GLFWwindow* window, const Device* const device,
	                           const RenderPass* const renderPass, uint32_t imageCount)
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
		m_descriptorPool =
		    vzt::DescriptorPool(device, descriptorTypes, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(window, true);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance                  = instance->vkHandle();
		init_info.PhysicalDevice            = device->getPhysicalDevice()->vkHandle();
		init_info.Device                    = device->vkHandle();

		QueueFamilyIndices indices = device->getDeviceQueueFamilyIndices();
		init_info.QueueFamily      = indices.graphicsFamily.value();
		init_info.Queue            = device->getGraphicsQueue();
		// init_info.PipelineCache             = g_PipelineCache;
		init_info.DescriptorPool = m_descriptorPool.vkHandle();
		init_info.Subpass        = 0;
		init_info.MinImageCount  = imageCount;
		init_info.ImageCount     = imageCount;
		init_info.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
		// init_info.Allocator                 = nullptr;
		// init_info.CheckVkResultFn           = nullptr;
		ImGui_ImplVulkan_Init(&init_info, renderPass->vkHandle());

		device->singleTimeCommand(
		    [](VkCommandBuffer commandBuffer) { ImGui_ImplVulkan_CreateFontsTexture(commandBuffer); });

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		ImGui_ImplVulkan_SetMinImageCount(imageCount);

		m_isInitialized = true;
	}

	void UiRenderer::record(VkCommandBuffer commandBuffer, const ui::UiManager& manager)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		manager.draw();

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
	}
} // namespace vzt
