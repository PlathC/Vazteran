#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/gtc/random.hpp>

#include "Vazteran/Application.hpp"
#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Scene.hpp"

int main(int /* args */, char*[] /* argv */)
{
	try
	{
		auto app = vzt::Application("Vazteran", vzt::Scene::Default(vzt::Scene::DefaultScene::CrounchingBoys));
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}