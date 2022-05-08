#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/Renderer.hpp"
#include "Vazteran/Backend/Vulkan/UiRenderer.hpp"
#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"
#include "Vazteran/Window.hpp"

int main(int /* args */, char*[] /* argv */)
{
	try
	{
		vzt::Window     window{"Vazteran", 800, 600};
		vzt::Renderer   renderer{&window};
		vzt::UiRenderer uiRenderer{};

		// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::CrounchingBoys);
		vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::VikingRoom);
		// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::MoriKnob);

		vzt::DescriptorLayout meshDescriptorLayout{};
		meshDescriptorLayout.addBinding(vzt::ShaderStage::VertexShader, 0, vzt::DescriptorType::UniformBuffer);
		meshDescriptorLayout.addBinding(vzt::ShaderStage::FragmentShader, 1, vzt::DescriptorType::UniformBuffer);
		meshDescriptorLayout.addBinding(vzt::ShaderStage::FragmentShader, 2, vzt::DescriptorType::CombinedSampler);

		vzt::Program triangleProgram{};
		triangleProgram.setShader(vzt::Shader("./shaders/triangle.vert.spv", vzt::ShaderStage::VertexShader));
		triangleProgram.setShader(vzt::Shader("./shaders/triangle.frag.spv", vzt::ShaderStage::FragmentShader));
		auto geometryPipeline = std::make_unique<vzt::GraphicPipeline>(
		    std::move(triangleProgram), meshDescriptorLayout,
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
		geometryBuffer.addColorOutput(normal, "Normal");
		geometryBuffer.addColorOutput(albedo, "Albedo");
		geometryBuffer.setDepthStencilOutput(depth, "Depth");
		geometryBuffer.setConfigureFunction([&](vzt::PipelineContextSettings settings) {
			meshView.configure(settings.device, renderer.getImageCount());
			geometryPipeline->configure(std::move(settings));
		});

		geometryBuffer.setRecordFunction([&](uint32_t imageId, const VkCommandBuffer& cmd,
		                                     const std::vector<VkDescriptorSet>& engineDescriptorSets) {
			geometryPipeline->bind(cmd);
			if (!engineDescriptorSets.empty())
			{
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipeline->layout(), 0,
				                        static_cast<uint32_t>(engineDescriptorSets.size()), engineDescriptorSets.data(),
				                        0, nullptr);
			}

			meshView.record(imageId, cmd, geometryPipeline.get());
		});

		vzt::Program fsBlinnPhongProgram = vzt::Program{};
		fsBlinnPhongProgram.setShader(vzt::Shader("./shaders/fs_triangle.vert.spv", vzt::ShaderStage::VertexShader));
		fsBlinnPhongProgram.setShader(vzt::Shader("./shaders/blinn_phong.frag.spv", vzt::ShaderStage::FragmentShader));
		vzt::GraphicPipeline compositionPipeline{std::move(fsBlinnPhongProgram)};
		compositionPipeline.getRasterOptions().cullMode = vzt::CullMode::Front;

		vzt::AttachmentHandle composed   = renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment});
		vzt::AttachmentHandle finalDepth = renderGraph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

		auto& deferredPass = renderGraph.addPass("Shading", vzt::QueueType::Graphic);
		deferredPass.addColorInput(position, "G-Position");
		deferredPass.addColorInput(normal, "G-Normal");
		deferredPass.addColorInput(albedo, "G-Albedo");
		deferredPass.setDepthStencilOutput(finalDepth, "Final Depth");
		deferredPass.addColorOutput(composed, "Composed");
		deferredPass.setConfigureFunction([&](vzt::PipelineContextSettings settings) {
			uiRenderer.configure(window.getInstance(), window.getWindowHandle(), settings.device,
			                     settings.renderPassTemplate, renderer.getImageCount());
			compositionPipeline.configure(std::move(settings));
		});

		deferredPass.setRecordFunction([&](uint32_t /* imageId */, const VkCommandBuffer& cmd,
		                                   const std::vector<VkDescriptorSet>& engineDescriptorSets) {
			compositionPipeline.bind(cmd);
			if (!engineDescriptorSets.empty())
			{
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, compositionPipeline.layout(), 0,
				                        static_cast<uint32_t>(engineDescriptorSets.size()), engineDescriptorSets.data(),
				                        0, nullptr);
			}

			vkCmdDraw(cmd, 3, 1, 0, 0);

			uiRenderer.record(cmd, *currentScene.sceneUi());
		});

		renderGraph.setBackBuffer(composed);
		renderGraph.compile();
		renderer.configure(renderGraph);

		renderer.setRenderFunction(
		    [&](uint32_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete, VkFence inFlightFence) {
			    renderGraph.render(imageId, imageAvailable, renderComplete, inFlightFence);
		    });

		const auto size                        = window.getFrameBufferSize();
		currentScene.sceneCamera().aspectRatio = size.width / static_cast<float>(size.height);

		static bool isMouseEnable = false;

		window.setOnFrameBufferChangedCallback([&]() {
			renderer.resize(window.getFrameBufferSize());
			renderer.configure(renderGraph);

			const auto size = window.getFrameBufferSize();

			currentScene.sceneCamera().aspectRatio = size.width / static_cast<float>(size.height);
		});
		window.setOnMousePosChangedCallback([&](vzt::Dvec2 deltaPos) {
			if (!isMouseEnable)
				return;

			currentScene.sceneCamera().update(deltaPos);
		});

		window.setOnKeyActionCallback([&](vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			float cameraSpeed = 5e-2f;
			auto& camera      = currentScene.sceneCamera();

			if ((modifiers & vzt::KeyModifier::Shift) == vzt::KeyModifier::Shift)
			{
				cameraSpeed *= 5.f;
			}

			if (code == vzt::KeyCode::W)
			{
				camera.position += camera.front * cameraSpeed;
			}
			else if (code == vzt::KeyCode::S)
			{
				camera.position -= camera.front * cameraSpeed;
			}
			else if (code == vzt::KeyCode::A)
			{
				camera.position -= glm::normalize(glm::cross(camera.front, camera.upVector)) * cameraSpeed;
			}
			else if (code == vzt::KeyCode::D)
			{
				camera.position += glm::normalize(glm::cross(camera.front, camera.upVector)) * cameraSpeed;
			}
		});

		window.setOnMouseButtonCallback([](vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			if (code == vzt::MouseButton::Left)
			{
				isMouseEnable = action == vzt::KeyAction::Press || action == vzt::KeyAction::Repeat;
			}
		});

		currentScene     = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::VikingRoom);
		auto sceneModels = currentScene.cModels();
		for (const auto* model : sceneModels)
		{
			meshView.addModel(model);
		}
		meshView.update(currentScene.cSceneCamera());

		while (!window.update())
		{
			currentScene.update();
			meshView.update(currentScene.cSceneCamera());
			renderer.render();
		}

		renderer.synchronize();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
