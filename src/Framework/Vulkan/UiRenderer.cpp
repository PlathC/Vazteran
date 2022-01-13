#include <array>
#include <stdexcept>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Vazteran/Framework/Vulkan/Instance.hpp"
#include "Vazteran/Framework/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Framework/Vulkan/RenderPass.hpp"
#include "Vazteran/Framework/Vulkan/UiRenderer.hpp"

namespace vzt
{
	UiRenderer::UiRenderer(GLFWwindow *window, Instance *instance, LogicalDevice *device, RenderPass *renderPass)
	    : m_device(device)
	{
		// 1: create descriptor pool for IMGUI
		//  the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] = {
		    {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
		    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
		    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
		    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
		    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
		    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
		    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
		    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
		    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
		    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
		    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(m_device->VkHandle(), &pool_info, nullptr, &m_descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Can't allocate descriptor pool.");
		}

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		// this initializes imgui for SDL
		ImGui_ImplGlfw_InitForVulkan(window, true);

		// this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = instance->VkHandle();
		init_info.PhysicalDevice = m_device->ChosenPhysicalDevice()->VkHandle();
		init_info.Device = m_device->VkHandle();
		init_info.Queue = m_device->GraphicsQueue();
		init_info.DescriptorPool = m_descriptorPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info, renderPass->VkHandle());

		// execute a gpu command to upload imgui font textures
		m_device->SingleTimeCommand(
		    [](VkCommandBuffer commandBuffer) { ImGui_ImplVulkan_CreateFontsTexture(commandBuffer); });

		// clear font textures from cpu data
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void UiRenderer::Update() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
		// ImGui::Checkbox("Demo Window", nullptr);      // Edit bools storing our window open/close state
		// ImGui::Checkbox("Another Window", nullptr);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		// ImGui::ColorEdit3("clear color", &clear_color); // Edit 3 floats representing a color

		if (ImGui::Button(
		        "Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text(
		    "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
		    ImGui::GetIO().Framerate);
		ImGui::End();

		ImGui::Render();
		// ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);
	}

	UiRenderer::~UiRenderer()
	{
		vkDestroyDescriptorPool(m_device->VkHandle(), m_descriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	}
} // namespace vzt
