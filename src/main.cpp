#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Backend/Vulkan/Renderer.hpp"
#include "Vazteran/Controller/FreeFly.hpp"
#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Renderer/ShaderLibrary.hpp"
#include "Vazteran/System/Scene.hpp"
#include "Vazteran/View/CompositionView.hpp"
#include "Vazteran/View/MeshView.hpp"
#include "Vazteran/View/UiView.hpp"
#include "Vazteran/Window.hpp"
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

		vzt::ui::MainMenuField fileMenuField = vzt::ui::MainMenuField("File");
		fileMenuField.addItem(vzt::ui::MainMenuItem("Open", []() { vzt::VZT_INFO("Open"); }));

		vzt::ui::MainMenuField brdfMenuField = vzt::ui::MainMenuField("BRDF");
		brdfMenuField.addItem(vzt::ui::MainMenuItem("Blinn-Phong", []() { vzt::VZT_INFO("Blinn-Phong"); }));

		vzt::ui::MainMenuBar mainMenuBar;
		mainMenuBar.addMenu(fileMenuField);
		mainMenuBar.addMenu(brdfMenuField);

		vzt::ui::UiManager uiManager;
		uiManager.setMainMenuBar(mainMenuBar);

		vzt::ShaderLibrary   library{};
		vzt::GraphicPipeline compositionPipeline;
		vzt::RenderGraph     renderGraph{};

		vzt::AttachmentHandle position =
		    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
		vzt::AttachmentHandle albedo =
		    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R16G16B16A16SFloat});
		vzt::AttachmentHandle normal =
		    renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::R8G8B8A8UNorm});
		vzt::AttachmentHandle depth = renderGraph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

		vzt::MeshView meshView{
		    renderer.getImageCount(), currentScene, library, renderGraph, position, normal, albedo, depth};

		vzt::AttachmentHandle composed      = renderGraph.addAttachment({vzt::ImageUsage::ColorAttachment});
		vzt::AttachmentHandle composedDepth = renderGraph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

		vzt::CompositionView compositionView{renderer.getImageCount(),
		                                     currentScene,
		                                     library,
		                                     renderGraph,
		                                     position,
		                                     normal,
		                                     albedo,
		                                     composedDepth,
		                                     composed};

		vzt::AttachmentHandle finalDepth = renderGraph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});
		vzt::UiView           uiRenderer{window, renderer.getImageCount(), renderGraph, finalDepth, composed};
		uiRenderer.setManager(uiManager);

		renderGraph.setBackBuffer(composed);
		renderGraph.compile();
		renderer.setRenderGraph(std::move(renderGraph));

		const auto   size       = window.getFrameBufferSize();
		vzt::Entity  mainCamera = currentScene.getMainCamera();
		vzt::Camera& camera     = mainCamera.get<vzt::Camera>();
		camera.aspectRatio      = size.x / static_cast<float>(size.y);
		mainCamera.emplace<vzt::FreeFly>(window, mainCamera);

		mainCamera.emplace<vzt::DynamicListener<vzt::Inputs>>(window, [&](const vzt::Inputs& inputs) {
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

		while (!window.update())
			renderer.render();

		renderer.synchronize();
	}
	catch (const std::exception& e)
	{
		vzt::VZT_ERROR(e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
