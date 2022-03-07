#ifndef VAZTERAN_VKUIRENDERER_HPP
#define VAZTERAN_VKUIRENDERER_HPP

#include <vector>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/GpuObjects.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Ui/UiManager.hpp"

namespace vzt
{
	class Device;
	class GraphicPipeline;
	class Instance;

	class UiRenderer
	{
	  public:
		UiRenderer(vzt::Instance* instance, vzt::Device* device, GLFWwindow* window, uint32_t imageCount,
		           const vzt::RenderPass* const renderPass, vzt::ui::UiManager uiManager);

		UiRenderer(const UiRenderer&) = delete;
		UiRenderer& operator=(const UiRenderer&) = delete;

		UiRenderer(UiRenderer&& other) noexcept = default;
		UiRenderer& operator=(UiRenderer&& other) noexcept = default;

		~UiRenderer();

		void record(VkCommandBuffer commandBuffer);

	  private:
		vzt::Device*        m_device;
		vzt::DescriptorPool m_descriptorPool;

		const vzt::ui::UiManager m_uiManager;

		uint32_t m_imageCount = 0;
	};
} // namespace vzt

#endif // VAZTERAN_VKUIRENDERER_HPP
