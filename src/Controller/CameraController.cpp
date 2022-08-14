#include "Vazteran/Controller/CameraController.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Window/Window.hpp"

namespace vzt
{
	CameraController::CameraController(Window& window, Entity cameraEntity) : m_cameraEntity(cameraEntity)
	{
		m_resizeConnection = window.subscribe<FrameBufferResize, &CameraController::operator()>(*this);
	}

	void CameraController::operator()(const FrameBufferResize& fbResize)
	{
		m_cameraEntity.patch<Camera>([&fbResize](Camera& camera) {
			camera.aspectRatio = fbResize.size.x / static_cast<float>(fbResize.size.y);
		});

		m_cameraEntity.patch<MainCamera>();
	}
} // namespace vzt
