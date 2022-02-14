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

	vzt::StorageHandle particlePositions = renderGraph.addStorage({128, vzt::BufferUsage::VertexBuffer});

	vzt::AttachmentHandle position = renderGraph.addAttachment({vzt::Format::R16G16B16A16SFloat});
	vzt::AttachmentHandle albedo   = renderGraph.addAttachment({vzt::Format::R16G16B16A16SFloat});
	vzt::AttachmentHandle normal   = renderGraph.addAttachment({vzt::Format::R8G8B8A8UNorm});
	vzt::AttachmentHandle depth    = renderGraph.addAttachment({});

	auto& computePass = renderGraph.addPass("Particle Generation", vzt::QueueType::Compute);
	computePass.addStorageOutput(particlePositions, "Particles");

	auto& geometryBuffer = renderGraph.addPass("G-Buffer", vzt::QueueType::Graphic);
	geometryBuffer.addStorageInput(particlePositions, "Particles");
	geometryBuffer.addColorOutput(position, "Position");
	geometryBuffer.addColorOutput(albedo, "Albedo");
	geometryBuffer.addColorOutput(normal, "Normal");
	geometryBuffer.setDepthStencilOutput(depth, "Depth");

	vzt::AttachmentHandle composed = renderGraph.addAttachment({});

	auto& deferredPass = renderGraph.addPass("Shading", vzt::QueueType::Graphic);
	deferredPass.addStorageInput(particlePositions, "Particles");
	deferredPass.addAttachmentInput(position, "G-Position");
	deferredPass.addAttachmentInput(albedo, "G-Albedo");
	deferredPass.addAttachmentInput(normal, "G-Normal");
	deferredPass.setDepthStencilInput(depth, "G-Depth");

	deferredPass.addColorOutput(composed, "Composed");

	auto& uiPass = renderGraph.addPass("UI", vzt::QueueType::Graphic);
	uiPass.addColorOutput(composed, "Final");

	renderGraph.setBackBuffer(composed);

	try
	{
		renderGraph.compile(vzt::Format::R8G8B8A8SRGB, vzt::Format::R8G8B8SNorm, vzt::Size2D<uint32_t>{1920, 1080});
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
