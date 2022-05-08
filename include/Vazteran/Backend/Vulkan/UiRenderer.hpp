#ifndef VAZTERAN_VKUIRENDERER_HPP
#define VAZTERAN_VKUIRENDERER_HPP

#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Ui/UiManager.hpp"

struct GLFWwindow;

namespace vzt
{
	class Device;
	class GraphicPipeline;
	class Instance;

	class UiRenderer
	{
	  public:
		UiRenderer() = default;

		UiRenderer(const UiRenderer&)            = delete;
		UiRenderer& operator=(const UiRenderer&) = delete;

		UiRenderer(UiRenderer&& other) noexcept;
		UiRenderer& operator=(UiRenderer&& other) noexcept;

		~UiRenderer();

		void configure(const Instance* const instance, GLFWwindow* window, const Device* const device,
		               const RenderPass* const renderPass, uint32_t imageCount);
		void record(VkCommandBuffer commandBuffer, const ui::UiManager& manager);

	  private:
		vzt::DescriptorPool m_descriptorPool;
		bool                m_isInitialized = false;
	};
} // namespace vzt

#endif // VAZTERAN_VKUIRENDERER_HPP
