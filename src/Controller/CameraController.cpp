#include "Vazteran/Controller/CameraController.hpp"

namespace vzt
{
	CameraController::CameraController(Window& window, Camera& camera)
	{
		m_resizeConnection = window.subscribe<Window::FrameBufferResize, &CameraController::operator()>(*this);
	}

	void CameraController::operator()(const Window::FrameBufferResize& fbResize)
	{
		m_camera->aspectRatio = fbResize.size.x / static_cast<float>(fbResize.size.y);
	}

} // namespace vzt
