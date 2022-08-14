#ifndef VAZTERAN_COMPOSITIONVIEW_HPP
#define VAZTERAN_COMPOSITIONVIEW_HPP

#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/System/System.hpp"
#include "Vazteran/View/View.hpp"

namespace vzt
{
	class Device;
	class RenderGraph;
	class Scene;
	class ShaderLibrary;

	class CompositionView : public View
	{
	  public:
		CompositionView(uint32_t imageNb, Scene& scene, ShaderLibrary& library);

		CompositionView(const CompositionView&)            = delete;
		CompositionView& operator=(const CompositionView&) = delete;

		CompositionView(CompositionView&& other) noexcept            = default;
		CompositionView& operator=(CompositionView&& other) noexcept = default;

		~CompositionView() = default;

		void apply(RenderGraph& graph, AttachmentHandle position, AttachmentHandle normal, AttachmentHandle albedo,
		           AttachmentHandle& depth, AttachmentHandle& composed);
		void refresh() override;
		void configure(const PipelineContextSettings& settings) override;
		void record(uint32_t imageId, VkCommandBuffer cmd,
		            const std::vector<VkDescriptorSet>& engineDescriptorSets) const override;

	  private:
		void createPipeline();

		ShaderLibrary*  m_library;
		Scene*          m_scene;
		GraphicPipeline m_pipeline;
	};
} // namespace vzt

#endif // VAZTERAN_COMPOSITIONVIEW_HPP
