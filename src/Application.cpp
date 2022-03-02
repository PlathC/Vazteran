#include <memory>
#include <random>
#include <stdexcept>

#include "Vazteran/Application.hpp"
#include "Vazteran/Backend/Vulkan/VkRenderer.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	Application::Application(const std::string& name, vzt::Scene scene) : m_scene(std::move(scene))
	{
		m_window = std::make_unique<vzt::Window>(name, 800, 600, [&]() {
			if (m_renderer)
			{
				m_renderer->resize(m_window->getFrameBufferSize());
				const auto size = m_window->getFrameBufferSize();

				m_scene.sceneCamera().aspectRatio = size.width / static_cast<float>(size.height);
			}
		});

		m_instance = std::make_unique<vzt::Instance>(name, m_window->vkExtensions());

		const auto size                   = m_window->getFrameBufferSize();
		m_scene.sceneCamera().aspectRatio = size.width / static_cast<float>(size.height);

		m_renderer =
		    std::make_unique<vzt::Renderer>(m_instance.get(), m_window->windowHandle(),
		                                    m_window->getSurface(m_instance.get()), m_window->getFrameBufferSize());
		m_renderer->setScene(&m_scene);

		static bool isMouseEnable = false;
		m_window->setOnMousePosChangedCallback([&](vzt::Dvec2 deltaPos) {
			if (!isMouseEnable)
				return;

			m_scene.sceneCamera().update(deltaPos);
		});

		m_window->setOnKeyActionCallback([&](vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			float cameraSpeed = 5e-2f;
			auto& camera      = m_scene.sceneCamera();

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

		m_window->setOnMouseButtonCallback(
		    [](vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			    if (code == vzt::MouseButton::Left)
			    {
				    isMouseEnable = action == vzt::KeyAction::Press || action == vzt::KeyAction::Repeat;
			    }
		    });
	}

	void Application::run()
	{
		while (!m_window->update())
		{
			m_scene.update();
			m_renderer->draw(m_scene.sceneCamera());
		}

		vkDeviceWaitIdle(m_renderer->getDevice()->vkHandle());
	}

	Application::~Application() { glfwTerminate(); }
} // namespace vzt
