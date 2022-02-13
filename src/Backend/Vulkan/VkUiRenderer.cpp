#include "Vazteran/Backend/Vulkan/VkUiRenderer.hpp"
#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/FrameBuffer.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/Instance.hpp"

namespace vzt
{
	VkUiRenderer::VkUiRenderer(vzt::Instance* instance, vzt::Device* device, GLFWwindow* window, uint32_t imageCount,
	                           const vzt::RenderPass* const renderPass, vzt::ui::UiManager uiManager)
	    : m_device(device), m_imageCount(imageCount), m_commandPool(m_device), m_uiManager(std::move(uiManager))
	{
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
		    vzt::DescriptorPool(m_device, descriptorTypes, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance                  = instance->vkHandle();
		init_info.PhysicalDevice            = m_device->getPhysicalDevice()->vkHandle();
		init_info.Device                    = m_device->VkHandle();

		QueueFamilyIndices indices = m_device->getDeviceQueueFamilyIndices();
		init_info.QueueFamily      = indices.graphicsFamily.value();
		init_info.Queue            = m_device->getGraphicsQueue();
		// init_info.PipelineCache             = g_PipelineCache;
		init_info.DescriptorPool = m_descriptorPool.vkHandle();
		init_info.Subpass        = 0;
		init_info.MinImageCount  = imageCount;
		init_info.ImageCount     = imageCount;
		init_info.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
		// init_info.Allocator                 = nullptr;
		// init_info.CheckVkResultFn           = nullptr;
		ImGui_ImplVulkan_Init(&init_info, renderPass->vkHandle());

		m_device->singleTimeCommand(
		    [](VkCommandBuffer commandBuffer) { ImGui_ImplVulkan_CreateFontsTexture(commandBuffer); });

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		ImGui_ImplVulkan_SetMinImageCount(imageCount);

		m_commandPool.allocateCommandBuffers(imageCount);
	}

	VkUiRenderer::~VkUiRenderer()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VkUiRenderer::record(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		m_uiManager.draw();

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();

		ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
	}
} // namespace vzt
