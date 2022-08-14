#include "Vazteran/View/CompositionView.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Renderer/ShaderLibrary.hpp"

namespace vzt
{
	CompositionView::CompositionView(uint32_t imageNb, Scene& scene, ShaderLibrary& library)
	    : View(imageNb), m_scene(&scene), m_library(&library)
	{
		createPipeline();
	}

	CompositionView::CompositionView(uint32_t imageNb, Scene& scene, ShaderLibrary& library, RenderGraph& graph,
	                                 AttachmentHandle position, AttachmentHandle normal, AttachmentHandle albedo,
	                                 AttachmentHandle& depth, AttachmentHandle& composed)
	    : CompositionView(imageNb, scene, library)
	{
		auto& deferredPass = graph.addPass("Shading", vzt::QueueType::Graphic);
		deferredPass.addColorInput(position, "G-Position");
		deferredPass.addColorInput(normal, "G-Normal");
		deferredPass.addColorInput(albedo, "G-Albedo");
		deferredPass.setDepthStencilOutput(depth, "Final Depth");
		deferredPass.addColorOutput(composed, "Composed");

		deferredPass.setConfigureFunction(
		    [this](const vzt::PipelineContextSettings& settings) { configure(settings); });

		deferredPass.setRecordFunction(
		    [this](uint32_t imageId, VkCommandBuffer cmd, const std::vector<VkDescriptorSet>& engineDescriptorSets) {
			    record(imageId, cmd, engineDescriptorSets);
		    });
	}

	void CompositionView::refresh() { createPipeline(); }

	void CompositionView::configure(const PipelineContextSettings& settings) { m_pipeline.configure(settings); }

	void CompositionView::record(uint32_t imageId, VkCommandBuffer cmd,
	                             const std::vector<VkDescriptorSet>& engineDescriptorSets) const
	{
		m_pipeline.bind(cmd);
		if (!engineDescriptorSets.empty())
			m_pipeline.bind(cmd, engineDescriptorSets);

		vkCmdDraw(cmd, 3, 1, 0, 0);
	}

	void CompositionView::createPipeline()
	{
		vzt::Program fsBlinnPhongProgram{};
		fsBlinnPhongProgram.setShader(m_library->get("./shaders/fs_triangle.vert"));
		fsBlinnPhongProgram.setShader(m_library->get("./shaders/blinn_phong.frag"));

		m_pipeline                             = vzt::GraphicPipeline{std::move(fsBlinnPhongProgram)};
		m_pipeline.getRasterOptions().cullMode = vzt::CullMode::Front;
	}

} // namespace vzt
