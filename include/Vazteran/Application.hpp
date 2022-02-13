
#ifndef VAZTERAN_APPLICATION_HPP
#define VAZTERAN_APPLICATION_HPP

#include <functional>
#include <string>

#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Scene.hpp"

namespace vzt
{
	class Instance;
	class Renderer;
	class UiRenderer;
	class Window;

	class Application
	{
	  public:
		explicit Application(const std::string& name, vzt::Scene scene);

		void run();

		~Application();

	  private:
		std::unique_ptr<vzt::Instance> m_instance;
		std::unique_ptr<vzt::Window>   m_window;
		std::unique_ptr<vzt::Renderer> m_renderer;

		Scene m_scene;
	};
} // namespace vzt

#endif // VAZTERAN_APPLICATION_HPP
