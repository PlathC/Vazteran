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
	/*
	 * try
	{
	    // auto app = vzt::Application("Vazteran", vzt::Scene::default(vzt::Scene::DefaultScene::CrounchingBoys));
	    // auto app = vzt::Application("Vazteran", vzt::Scene::default(vzt::Scene::DefaultScene::VikingRoom));
	    auto app = vzt::Application("Vazteran", vzt::Scene::default(vzt::Scene::DefaultScene::CrounchingBoys)
	                                // vzt::Scene::default(vzt::Scene::DefaultScene::VikingRoom)
	                                // vzt::Scene::default(vzt::Scene::DefaultScene::MoriKnob)
	    );
	    app.run();
	}
	catch (const std::exception& e)
	{
	    std::cerr << e.what() << std::endl;
	    return EXIT_FAILURE;
	}
	*/

	vzt::RenderGraph renderGraph{};

	const vzt::AttachmentHandle position = renderGraph.addAttachment({vzt::Format::R16G16B16A16SFloat});
	const vzt::AttachmentHandle albedo   = renderGraph.addAttachment({vzt::Format::R16G16B16A16SFloat});
	const vzt::AttachmentHandle normal   = renderGraph.addAttachment({vzt::Format::R8G8B8A8UNorm});
	const vzt::AttachmentHandle depth    = renderGraph.addAttachment({});

	auto& geometryBuffer = renderGraph.addPass("G-Buffer", vzt::QueueType::Graphic);
	geometryBuffer.addColorOutput(position, "Position");
	geometryBuffer.addColorOutput(albedo, "Albedo");
	geometryBuffer.addColorOutput(normal, "Normal");
	geometryBuffer.setDepthStencilOutput(depth, "Depth");

	auto& deferredPass = renderGraph.addPass("Shading", vzt::QueueType::Graphic);
	deferredPass.addAttachmentInput(position, "G-Position");
	deferredPass.addAttachmentInput(albedo, "G-Albedo");
	deferredPass.addAttachmentInput(normal, "G-Normal");
	deferredPass.setDepthStencilInput(depth, "G-Depth");

	const vzt::AttachmentHandle composed = renderGraph.addAttachment({});
	deferredPass.addColorOutput(composed, "Composed");

	auto& uiPass = renderGraph.addPass("UI", vzt::QueueType::Graphic);
	uiPass.addColorOutput(composed, "Final");

	renderGraph.setBackBuffer(composed);

	renderGraph.compile(vzt::Format::R8G8B8A8SRGB, vzt::Format::R8G8B8SNorm, vzt::Size2D<uint32_t>{1920, 1080});

	return EXIT_SUCCESS;
}
