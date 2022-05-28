#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/Renderer.hpp"
#include "Vazteran/Backend/Vulkan/UiRenderer.hpp"
#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Renderer/ShaderLibrary.hpp"
#include "Vazteran/System/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"
#include "Vazteran/Window.hpp"

int main(int /* args */, char*[] /* argv */)
{
	try
	{
		vzt::Window     window{"Vazteran", 1280, 720};
		vzt::Renderer   renderer{&window};
		vzt::UiRenderer uiRenderer{};

		// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::CrounchingBoys);
		vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::VikingRoom);
		// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::MoriKnob);

		vzt::ui::MainMenuField fileMenuField = vzt::ui::MainMenuField("File");
		fileMenuField.addItem(vzt::ui::MainMenuItem("Open", []() { vzt::VZT_INFO("Open"); }));

		vzt::ui::MainMenuField brdfMenuField = vzt::ui::MainMenuField("BRDF");
		brdfMenuField.addItem(vzt::ui::MainMenuItem("Blinn-Phong", []() { vzt::VZT_INFO("Blinn-Phong"); }));

		vzt::ui::MainMenuBar mainMenuBar;
		mainMenuBar.addMenu(fileMenuField);
		mainMenuBar.addMenu(brdfMenuField);

		vzt::ui::UiManager uiManager;
		uiManager.setMainMenuBar(mainMenuBar);

		vzt::DescriptorLayout meshDescriptorLayout{};
		meshDescriptorLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
		meshDescriptorLayout.addBinding(1, vzt::DescriptorType::UniformBuffer);
		meshDescriptorLayout.addBinding(2, vzt::DescriptorType::CombinedSampler);

		vzt::ShaderLibrary   library{};
		vzt::GraphicPipeline geometryPipeline;
		vzt::GraphicPipeline compositionPipeline;
		vzt::RenderGraph     renderGraph{};
		renderer.setRenderFunction(
		    [&](uint32_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete, VkFence inFlightFence) {
			    renderGraph.render(imageId, imageAvailable, renderComplete, inFlightFence);
		    });

		vzt::Program triangleProgram{};
		triangleProgram.setShader(library.get("./shaders/triangle.vert"));
		triangleProgram.setShader(library.get("./shaders/triangle.frag"));
		geometryPipeline = vzt::GraphicPipeline(std::move(triangleProgram), meshDescriptorLayout,
		                                        vzt::TriangleVertexInput::getInputDescription());

		vzt::MeshView meshView{currentScene};

		vzt::AttachmentHandle position =
		    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
		vzt::AttachmentHandle albedo =
		    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
		vzt::AttachmentHandle normal =
		    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R8G8B8A8UNorm});
		vzt::AttachmentHandle depth = renderGraph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

		{
			auto& geometryBuffer = renderGraph.addPass("G-Buffer", vzt::QueueType::Graphic);
			geometryBuffer.addColorOutput(position, "Position");
			geometryBuffer.addColorOutput(normal, "Normal");
			geometryBuffer.addColorOutput(albedo, "Albedo");
			geometryBuffer.setDepthStencilOutput(depth, "Depth");

			geometryBuffer.setConfigureFunction(
			    [&meshView, &geometryPipeline, &renderer](const vzt::RenderPassHandler& pass) {
				    meshView.configure(pass.getDevice(), renderer.getImageCount());
				    geometryPipeline.configure({pass.getDevice(), pass.getTemplate(), pass.getDescriptorLayout(),
				                                pass.getOutputAttachmentNb(), pass.getExtent()});
			    });

			geometryBuffer.setRecordFunction(
			    [&](uint32_t imageId, VkCommandBuffer cmd, const std::vector<VkDescriptorSet>& engineDescriptorSets) {
				    geometryPipeline.bind(cmd);
				    if (!engineDescriptorSets.empty())
					    geometryPipeline.bind(cmd, engineDescriptorSets);

				    meshView.record(imageId, cmd, &geometryPipeline);
			    });
		}

		vzt::Program fsBlinnPhongProgram{};
		fsBlinnPhongProgram.setShader(library.get("./shaders/fs_triangle.vert"));
		fsBlinnPhongProgram.setShader(library.get("./shaders/blinn_phong.frag"));

		compositionPipeline                             = vzt::GraphicPipeline{std::move(fsBlinnPhongProgram)};
		compositionPipeline.getRasterOptions().cullMode = vzt::CullMode::Front;

		vzt::AttachmentHandle composed   = renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment});
		vzt::AttachmentHandle finalDepth = renderGraph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

		{
			auto& deferredPass = renderGraph.addPass("Shading", vzt::QueueType::Graphic);
			deferredPass.addColorInput(position, "G-Position");
			deferredPass.addColorInput(normal, "G-Normal");
			deferredPass.addColorInput(albedo, "G-Albedo");
			deferredPass.setDepthStencilOutput(finalDepth, "Final Depth");
			deferredPass.addColorOutput(composed, "Composed");

			deferredPass.setConfigureFunction(
			    [&window, &uiRenderer, &compositionPipeline, &renderer](const vzt::RenderPassHandler& pass) {
				    uiRenderer.configure(window.getInstance(), window.getWindowHandle(), pass.getDevice(),
				                         pass.getTemplate(), renderer.getImageCount());
				    compositionPipeline.configure({pass.getDevice(), pass.getTemplate(), pass.getDescriptorLayout(),
				                                   pass.getOutputAttachmentNb(), pass.getExtent()});
			    });

			deferredPass.setRecordFunction([&](uint32_t /* imageId */, VkCommandBuffer cmd,
			                                   const std::vector<VkDescriptorSet>& engineDescriptorSets) {
				compositionPipeline.bind(cmd);
				if (!engineDescriptorSets.empty())
					compositionPipeline.bind(cmd, engineDescriptorSets);

				vkCmdDraw(cmd, 3, 1, 0, 0);

				uiRenderer.record(cmd, uiManager);
			});
		}

		renderGraph.setBackBuffer(composed);
		renderGraph.compile();
		renderer.configure(renderGraph);

		const auto size = window.getFrameBufferSize();

		currentScene.getMainCamera().get<vzt::Camera>().aspectRatio = size.width / static_cast<float>(size.height);

		static bool isMouseEnable = false;

		window.setOnFrameBufferChangedCallback([&]() {
			renderer.resize(window.getFrameBufferSize());
			renderer.configure(renderGraph);

			const auto size = window.getFrameBufferSize();

			currentScene.getMainCamera().get<vzt::Camera>().aspectRatio = size.width / static_cast<float>(size.height);
		});
		window.setOnMousePosChangedCallback([&](vzt::Dvec2 deltaPos) {
			if (!isMouseEnable)
				return;

			currentScene.getMainCamera().get<vzt::Camera>().update(deltaPos);
		});

		window.setOnKeyActionCallback([&](vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			float cameraSpeed = 5e-2f;
			auto& camera      = currentScene.getMainCamera().get<vzt::Camera>();

			if ((modifiers & vzt::KeyModifier::Shift) == vzt::KeyModifier::Shift)
				cameraSpeed *= 5.f;

			if (code == vzt::KeyCode::W)
				camera.position += camera.front * cameraSpeed;
			if (code == vzt::KeyCode::S)
				camera.position -= camera.front * cameraSpeed;
			if (code == vzt::KeyCode::A)
				camera.position -= glm::normalize(glm::cross(camera.front, camera.upVector)) * cameraSpeed;
			if (code == vzt::KeyCode::D)
				camera.position += glm::normalize(glm::cross(camera.front, camera.upVector)) * cameraSpeed;

			if (code == vzt::KeyCode::F8)
			{
				renderer.synchronize();

				library.reload();

				triangleProgram = vzt::Program{};
				triangleProgram.setShader(library.get("./shaders/triangle.vert"));
				triangleProgram.setShader(library.get("./shaders/triangle.frag"));
				geometryPipeline = vzt::GraphicPipeline(std::move(triangleProgram), meshDescriptorLayout,
				                                        vzt::TriangleVertexInput::getInputDescription());

				fsBlinnPhongProgram = vzt::Program{};
				fsBlinnPhongProgram.setShader(library.get("./shaders/fs_triangle.vert"));
				fsBlinnPhongProgram.setShader(library.get("./shaders/blinn_phong.frag"));

				compositionPipeline                             = vzt::GraphicPipeline{std::move(fsBlinnPhongProgram)};
				compositionPipeline.getRasterOptions().cullMode = vzt::CullMode::Front;

				renderer.configure(renderGraph);
			}
		});

		window.setOnMouseButtonCallback([](vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			if (code == vzt::MouseButton::Left)
				isMouseEnable = action == vzt::KeyAction::Press || action == vzt::KeyAction::Repeat;
		});

		while (!window.update())
		{
			meshView.update(currentScene.getMainCamera().get<vzt::Camera>());
			renderer.render();
		}

		renderer.synchronize();
	}
	catch (const std::exception& e)
	{
		vzt::VZT_ERROR(e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
