#include <memory>
#include <random>
#include <stdexcept>

#include "Vazteran/Application.hpp"
#include "Vazteran/Backend/Vulkan/VkRenderer.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	Application::Application(std::string_view name, vzt::Scene scene) : m_scene(std::move(scene))
	{
		m_window = std::make_unique<vzt::Window>(name, 800, 600, [&]() {
			if (m_renderer)
			{
				m_renderer->FrameBufferResized(m_window->FrameBufferSize());
				const auto size = m_window->FrameBufferSize();

				m_scene.SceneCamera().aspectRatio = size.width / static_cast<float>(size.height);
			}
		});

		m_instance = std::make_unique<vzt::Instance>(name, m_window->VkExtensions());

		const auto size                   = m_window->FrameBufferSize();
		m_scene.SceneCamera().aspectRatio = size.width / static_cast<float>(size.height);

		m_renderer = std::make_unique<vzt::Renderer>(m_instance.get(), m_window->Handle(),
		                                             m_window->Surface(m_instance.get()), m_window->FrameBufferSize());
		m_renderer->SetScene(&m_scene);

		static bool isMouseEnable = false;
		m_window->SetOnMousePosChangedCallback([&](vzt::Dvec2 deltaPos) {
			if (!isMouseEnable)
				return;

			static float    yaw         = 90.f;
			static float    pitch       = 0.f;
			constexpr float sensitivity = .5f;

			yaw += deltaPos.x * sensitivity;
			pitch += deltaPos.y * sensitivity;

			pitch = std::max(std::min(pitch, 89.f), -89.f);

			vzt::Vec3 direction;
			direction.x = -std::cos(vzt::ToRadians(yaw)) * std::cos(vzt::ToRadians(pitch));
			direction.y = std::sin(vzt::ToRadians(yaw)) * std::cos(vzt::ToRadians(pitch));
			direction.z = std::sin(vzt::ToRadians(pitch));

			m_scene.SceneCamera().front = glm::normalize(direction);
		});

		m_window->SetOnKeyActionCallback([&](vzt::KeyCode code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			float cameraSpeed = 5e-2f;
			auto& camera      = m_scene.SceneCamera();

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

		m_window->SetOnMouseButtonCallback(
		    [](vzt::MouseButton code, vzt::KeyAction action, vzt::KeyModifier modifiers) {
			    if (code == vzt::MouseButton::Left)
			    {
				    isMouseEnable = action == vzt::KeyAction::Press || action == vzt::KeyAction::Repeat;
			    }
		    });
	}

	void Application::Run()
	{
		while (!m_window->Update())
		{
			m_scene.Update();
			m_renderer->Draw(m_scene.SceneCamera());
		}

		vkDeviceWaitIdle(m_renderer->Device()->VkHandle());
	}

	Application::~Application() { glfwTerminate(); }
} // namespace vzt
