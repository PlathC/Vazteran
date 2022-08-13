#ifndef VAZTERAN_CAMERACONTROLLER_HPP
#define VAZTERAN_CAMERACONTROLLER_HPP

#include "Vazteran/Core/Event.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	class CameraController
	{
	  public:
		CameraController(Window& window, Camera& camera);
		~CameraController() = default;

		void operator()(const Window::FrameBufferResize& fbResize);

	  private:
		Camera*    m_camera;
		Connection m_resizeConnection;
	};
} // namespace vzt

#endif // VAZTERAN_CAMERACONTROLLER_HPP
