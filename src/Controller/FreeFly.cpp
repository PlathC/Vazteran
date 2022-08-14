#include "Vazteran/Controller/FreeFly.hpp"
#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Transform.hpp"
#include "Vazteran/Math/Math.hpp"
#include "Vazteran/System/Scene.hpp"
#include "Vazteran/Window/Window.hpp"

namespace vzt
{
	FreeFly::FreeFly(Window& window, Entity cameraEntity, KeyCode toggleEnable)
	    : CameraController(window, cameraEntity), m_toggleEnable(toggleEnable)
	{
		m_inputConnection = window.subscribe<InputHandler, &FreeFly::operator()>(*this);
	}

	void FreeFly::setEnable(bool enable) { m_enabled = enable; }
	bool FreeFly::getEnable() const { return m_enabled; }

	void FreeFly::operator()(const InputHandler& inputs)
	{
		constexpr float sensitivity     = 1e-2f;
		constexpr float baseCameraSpeed = 5e-3f;

		if (inputs.get(m_toggleEnable))
			m_enabled = !m_enabled;

		if (!m_enabled)
			return;

		Vec3 rotation{};
		if (inputs.get(MouseButton::Left))
		{
			rotation.x = -static_cast<float>(inputs.deltaMousePosition.y) * sensitivity;
			rotation.z = -static_cast<float>(inputs.deltaMousePosition.x) * sensitivity;
		}
		else if (inputs.get(MouseButton::Right))
		{
			rotation.z = -static_cast<float>(inputs.deltaMousePosition.x) * sensitivity;
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

		const bool updateTranslation = glm::dot(translation, translation) > 0.f;
		const bool updateRotation    = glm::dot(rotation, rotation) > 0.f;

		if (!updateTranslation && !updateRotation)
			return;

		auto& transform = m_cameraEntity.get<Transform>();
		if (updateTranslation)
			transform.translateRelative(translation);
		if (updateRotation)
			transform.rotate(rotation);

		m_cameraEntity.patch<MainCamera>();
	}

} // namespace vzt
