#include "Vazteran/Controller/FreeFly.hpp"
#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Transform.hpp"
#include "Vazteran/Math/Math.hpp"
#include "Vazteran/System/Scene.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	FreeFly::FreeFly(Window& window, Camera& camera, Transform& transform, KeyCode toggleEnable)
	    : CameraController(window, camera), m_transform(&transform), m_toggleEnable(toggleEnable)
	{
		m_inputConnection = window.subscribe<Inputs, &FreeFly::operator()>(*this);
	}

	void FreeFly::setEnable(bool enable) { m_enabled = enable; }
	bool FreeFly::getEnable() const { return m_enabled; }

	void FreeFly::operator()(const Inputs& inputs)
	{
		constexpr float sensitivity     = 1e-2f;
		constexpr float baseCameraSpeed = 5e-3f;

		if (inputs.get(m_toggleEnable))
			m_enabled = !m_enabled;

		if (!m_enabled)
			return;

		if (inputs.get(MouseButton::Left))
		{
			m_transform->rotate(Vec3(-static_cast<float>(inputs.deltaMousePosition.y) * sensitivity, 0.f,
			                         -static_cast<float>(inputs.deltaMousePosition.x) * sensitivity));
		}
		else if (inputs.get(MouseButton::Right))
		{
			m_transform->rotate(Vec3(0.f, -static_cast<float>(inputs.deltaMousePosition.x) * sensitivity, 0.f));
		}

		float cameraSpeed = baseCameraSpeed;
		if (inputs.get(KeyModifier::Shift))
			cameraSpeed *= 5.f;

		Vec3 translation{0.f};
		if (inputs.get(KeyCode::W))
			translation += Camera::Front * cameraSpeed;
		if (inputs.get(KeyCode::S))
			translation -= Camera::Front * cameraSpeed;

		if (inputs.get(KeyCode::D))
			translation += Camera::Right * cameraSpeed;
		if (inputs.get(KeyCode::A))
			translation -= Camera::Right * cameraSpeed;

		if (glm::dot(translation, translation) > 0.f)
			m_transform->translateRelative(translation);
	}

} // namespace vzt
