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

	try
	{
		// auto app = vzt::Application("Vazteran", vzt::Scene::defaultScene(vzt::Scene::DefaultScene::CrounchingBoys));
		// auto app = vzt::Application("Vazteran", vzt::Scene::defaultScene(vzt::Scene::DefaultScene::VikingRoom));
		auto app = vzt::Application("Vazteran", vzt::Scene::defaultScene(vzt::Scene::DefaultScene::CrounchingBoys)
		                            // vzt::Scene::defaultScene(vzt::Scene::DefaultScene::VikingRoom)
		                            // vzt::Scene::defaultScene(vzt::Scene::DefaultScene::MoriKnob)
		);
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
