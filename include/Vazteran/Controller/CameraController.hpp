#ifndef VAZTERAN_CAMERACONTROLLER_HPP
#define VAZTERAN_CAMERACONTROLLER_HPP

#include "Vazteran/Core/Event.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/System/System.hpp"
#include "Vazteran/Window.hpp"

namespace vzt
{
	class CameraController
	{
	  public:
		CameraController(Window& window, Entity cameraEntity);
		~CameraController() = default;

		void operator()(const Window::FrameBufferResize& fbResize);

	  protected:
		Entity m_cameraEntity;

	  private:
		Connection m_resizeConnection;
	};
} // namespace vzt

#endif // VAZTERAN_CAMERACONTROLLER_HPP
