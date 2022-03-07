#include <cstdlib>
#include <iostream>
#include <memory>

#include "Vazteran/Application.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"

int main(int /* args */, char*[] /* argv */)
{
	vzt::Program fsBlinnPhongProgram = vzt::Program{};
	fsBlinnPhongProgram.setShader(vzt::Shader("./shaders/fs_triangle.vert.spv", vzt::ShaderStage::VertexShader));
	fsBlinnPhongProgram.setShader(vzt::Shader("./shaders/blinn_phong.frag.spv", vzt::ShaderStage::FragmentShader));
	auto compositionPipeline = std::make_unique<vzt::GraphicPipeline>(std::move(fsBlinnPhongProgram));

	vzt::DescriptorLayout meshDescriptorLayout{};
	meshDescriptorLayout.addBinding(vzt::ShaderStage::VertexShader, 0, vzt::DescriptorType::UniformBuffer);
	meshDescriptorLayout.addBinding(vzt::ShaderStage::FragmentShader, 1, vzt::DescriptorType::UniformBuffer);
	meshDescriptorLayout.addBinding(vzt::ShaderStage::FragmentShader, 2, vzt::DescriptorType::CombinedSampler);

	vzt::Program triangleProgram{};
	triangleProgram.setShader(vzt::Shader("./shaders/triangle.vert.spv", vzt::ShaderStage::VertexShader));
	triangleProgram.setShader(vzt::Shader("./shaders/triangle.frag.spv", vzt::ShaderStage::FragmentShader));
	auto geometryPipeline = std::make_unique<vzt::GraphicPipeline>(
	    std::move(triangleProgram), std::optional<vzt::DescriptorLayout>{},
	    vzt::VertexInputDescription{vzt::TriangleVertexInput::getBindingDescription(),
	                                vzt::TriangleVertexInput::getAttributeDescription()});

	vzt::MeshView meshView{};

	vzt::RenderGraph      renderGraph{};
	vzt::AttachmentHandle position =
	    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
	vzt::AttachmentHandle albedo =
	    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
	vzt::AttachmentHandle normal =
	    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R8G8B8A8UNorm});
	vzt::AttachmentHandle depth = renderGraph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

	auto& geometryBuffer = renderGraph.addPass("G-Buffer", vzt::QueueType::Graphic);
	geometryBuffer.addColorOutput(position, "Position");
	geometryBuffer.addColorOutput(albedo, "Albedo");
	geometryBuffer.addColorOutput(normal, "Normal");
	geometryBuffer.setDepthStencilOutput(depth, "Depth");
	geometryBuffer.setConfigureFunction([&](uint32_t imageCount, vzt::PipelineContextSettings settings) {
		meshView.configure(settings.device, imageCount);
		geometryPipeline->configure(settings);
	});

	geometryBuffer.setRenderFunction([&](uint32_t imageId, const vzt::RenderPass* renderPass,
	                                     const VkCommandBuffer&              cmd,
	                                     const std::vector<VkDescriptorSet>& engineDescriptorSets) {
		geometryPipeline->bind(cmd, renderPass);
		if (!engineDescriptorSets.empty())
		{
			vkCmdBindDescriptorSets(cmd, vzt::toVulkan(vzt::PipelineBindPoint::Graphics), geometryPipeline->layout(), 0,
			                        static_cast<uint32_t>(engineDescriptorSets.size()), engineDescriptorSets.data(), 0,
			                        nullptr);
		}

		meshView.record(imageId, cmd);
	});

	vzt::AttachmentHandle composed = renderGraph.addAttachment({});

	auto& deferredPass = renderGraph.addPass("Shading", vzt::QueueType::Graphic);
	deferredPass.addColorInput(position, "G-Position");
	deferredPass.addColorInput(albedo, "G-Albedo");
	deferredPass.addColorInput(normal, "G-Normal");
	deferredPass.setDepthStencilOutput(depth, "G-Depth");
	deferredPass.addColorOutput(composed, "Composed");
	deferredPass.setConfigureFunction([&](uint32_t /* imageCount */, vzt::PipelineContextSettings settings) {
		compositionPipeline->configure(settings);
	});
	deferredPass.setRenderFunction([&](uint32_t /* imageId */, const vzt::RenderPass* renderPass,
	                                   const VkCommandBuffer&              cmd,
	                                   const std::vector<VkDescriptorSet>& engineDescriptorSets) {
		compositionPipeline->bind(cmd, renderPass);
		if (!engineDescriptorSets.empty())
		{
			vkCmdBindDescriptorSets(cmd, vzt::toVulkan(vzt::PipelineBindPoint::Graphics), compositionPipeline->layout(),
			                        0, static_cast<uint32_t>(engineDescriptorSets.size()), engineDescriptorSets.data(),
			                        0, nullptr);
		}

		vkCmdDraw(cmd, 3, 1, 0, 0);
	});

	// auto& uiPass = renderGraph.addPass("UI", vzt::QueueType::Graphic);
	// uiPass.addColorOutput(composed, "Final");
	// uiPass.setDepthStencilOutput(depth, "Depth");

	renderGraph.setBackBuffer(composed);

	try
	{
		renderGraph.compile();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::CrounchingBoys);
	// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::VikingRoom);
	// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::MoriKnob);

	try
	{
		auto app = vzt::Application("Vazteran", std::move(currentScene), std::move(renderGraph));
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
