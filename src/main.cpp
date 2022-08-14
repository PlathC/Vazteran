#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/Renderer.hpp"
#include "Vazteran/Controller/FreeFly.hpp"
#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Renderer/ShaderLibrary.hpp"
#include "Vazteran/System/Scene.hpp"
#include "Vazteran/View/CompositionView.hpp"
#include "Vazteran/View/MeshView.hpp"
#include "Vazteran/View/UiView.hpp"
#include "Vazteran/Window/Window.hpp"
#include <include/Vazteran/Data/Transform.hpp>

int main(int /* args */, char*[] /* argv */)
{
	try
	{
		vzt::Window   window{"Vazteran", 1280, 720};
		vzt::Renderer renderer{window};

		// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::CrounchingBoys);
		vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::VikingRoom);
		// vzt::Scene currentScene = vzt::Scene::defaultScene(vzt::Scene::DefaultScene::MoriKnob);

		vzt::ShaderLibrary   library{};
		vzt::GraphicPipeline compositionPipeline;

		vzt::MeshView        meshView{renderer.getImageCount(), currentScene, library};
		vzt::CompositionView compositionView{renderer.getImageCount(), currentScene, library};
		vzt::UiView          uiView{window, renderer.getImageCount()};

		// Rendergraph defintion
		vzt::RenderGraph graph{};

		vzt::AttachmentHandle position =
		    graph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
		vzt::AttachmentHandle albedo =
		    graph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
		vzt::AttachmentHandle normal =
		    graph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R8G8B8A8UNorm});
		vzt::AttachmentHandle depth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});
		meshView.apply(graph, position, normal, albedo, depth);

		vzt::AttachmentHandle composed      = graph.addAttachment({vzt::ImageUsage::ColorAttachment});
		vzt::AttachmentHandle composedDepth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});
		compositionView.apply(graph, position, normal, albedo, composedDepth, composed);

		vzt::AttachmentHandle finalDepth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});
		uiView.apply(graph, finalDepth, composed);

		graph.setBackBuffer(composed);
		graph.compile();
		renderer.setRenderGraph(std::move(graph));

		// Ui definition
		vzt::ui::MainMenuField fileMenuField = vzt::ui::MainMenuField("File");
		fileMenuField.addItem(vzt::ui::MainMenuItem("Open", []() { vzt::VZT_INFO("Open"); }));

		vzt::ui::MainMenuField brdfMenuField = vzt::ui::MainMenuField("BRDF");
		brdfMenuField.addItem(vzt::ui::MainMenuItem("Blinn-Phong", []() { vzt::VZT_INFO("Blinn-Phong"); }));

		vzt::ui::MainMenuBar mainMenuBar;
		mainMenuBar.addMenu(fileMenuField);
		mainMenuBar.addMenu(brdfMenuField);

		vzt::ui::UiManager uiManager;
		uiManager.setMainMenuBar(mainMenuBar);
		uiView.setManager(uiManager);

		// Callback definition
		const auto   size       = window.getFrameBufferSize();
		vzt::Entity  mainCamera = currentScene.getMainCamera();
		vzt::Camera& camera     = mainCamera.get<vzt::Camera>();
		camera.aspectRatio      = size.x / static_cast<float>(size.y);
		mainCamera.emplace<vzt::FreeFly>(window, mainCamera);

		mainCamera.emplace<vzt::DynamicListener<vzt::InputHandler>>(window, [&](const vzt::InputHandler& inputs) {
			if (inputs.get(vzt::KeyCode::F8))
			{
				// Stop rendering
				renderer.synchronize();

				// Refresh shaders
				library.reload();

				// Reload shaders from library
				meshView.refresh();
				compositionView.refresh();

				// Recreate full rendering pipeline
				renderer.refresh();
			}
		});

		// Rendering loop
		while (!window.update())
			renderer.render();

		// Don't forget to synchronize the CPU and GPU before closing
		// TODO: Make this automatic
		renderer.synchronize();
	}
	catch (const std::exception& e)
	{
		vzt::VZT_ERROR(e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
