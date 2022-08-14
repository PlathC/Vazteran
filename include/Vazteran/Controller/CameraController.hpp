#ifndef VAZTERAN_CAMERACONTROLLER_HPP
#define VAZTERAN_CAMERACONTROLLER_HPP

#include "Vazteran/Core/Event.hpp"
#include "Vazteran/System/System.hpp"

namespace vzt
{
	class Window;
	struct FrameBufferResize;

	class CameraController
	{
	  public:
		CameraController(Window& window, Entity cameraEntity);
		~CameraController() = default;

		void operator()(const FrameBufferResize& fbResize);

	  protected:
		Entity m_cameraEntity;

	  private:
		Connection m_resizeConnection;
	};
} // namespace vzt

#endif // VAZTERAN_CAMERACONTROLLER_HPP
