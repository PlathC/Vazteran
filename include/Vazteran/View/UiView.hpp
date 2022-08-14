#ifndef VAZTERAN_VULKAN_UIRENDERER_HPP
#define VAZTERAN_VULKAN_UIRENDERER_HPP

#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Ui/UiManager.hpp"
#include "Vazteran/View/View.hpp"

struct GLFWwindow;

namespace vzt
{
	struct AttachmentHandle;
	class Device;
	class GraphicPipeline;
	class Instance;
	class RenderGraph;
	class Window;

	class UiView : public View
	{
	  public:
		UiView(const Window& window, uint32_t imageCount);
		UiView(const Window& window, uint32_t imageCount, RenderGraph& graph, AttachmentHandle& depth,
		       AttachmentHandle& composed);

		UiView(const UiView&)            = delete;
		UiView& operator=(const UiView&) = delete;

		UiView(UiView&& other) noexcept;
		UiView& operator=(UiView&& other) noexcept;

		~UiView();

		void setManager(const ui::UiManager& manager);

		void refresh() override;
		void configure(const PipelineContextSettings& settings) override;
		void record(uint32_t imageId, VkCommandBuffer cmd,
		            const std::vector<VkDescriptorSet>& engineDescriptorSets) const override;

	  private:
		const Instance*      m_instance;
		const Window*        m_window;
		const ui::UiManager* m_manager = nullptr;

		DescriptorPool m_descriptorPool;
		bool           m_isInitialized = false;
	};
} // namespace vzt

#endif // VAZTERAN_VULKAN_UIRENDERER_HPP
