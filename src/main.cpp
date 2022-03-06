#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/gtc/random.hpp>

#include "Vazteran/Application.hpp"
#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Scene.hpp"

int main(int /* args */, char*[] /* argv */)
{
	vzt::RenderGraph renderGraph{};

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

	vzt::AttachmentHandle composed = renderGraph.addAttachment({});

	auto& deferredPass = renderGraph.addPass("Shading", vzt::QueueType::Graphic);
	deferredPass.addColorInput(position, "G-Position");
	deferredPass.addColorInput(albedo, "G-Albedo");
	deferredPass.addColorInput(normal, "G-Normal");
	deferredPass.setDepthStencilInput(depth, "G-Depth");
	deferredPass.addColorOutput(composed, "Composed");

	auto& uiPass = renderGraph.addPass("UI", vzt::QueueType::Graphic);
	uiPass.addColorOutput(composed, "Final");
	uiPass.setDepthStencilOutput(depth, "Depth");

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
