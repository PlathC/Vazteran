#ifndef VATERAN_FREEFLY_HPP
#define VATERAN_FREEFLY_HPP

#include "Vazteran/Controller/CameraController.hpp"
#include "Vazteran/Window/Input.hpp"

namespace vzt
{
	struct InputHandler;

	class FreeFly : public CameraController
	{
	  public:
		FreeFly(Window& window, Entity cameraEntity, KeyCode toggleEnable = KeyCode::Tab);
		~FreeFly() = default;

		void setEnable(bool enable);
		bool getEnable() const;
		void operator()(const InputHandler& inputs);

	  private:
		bool       m_enabled = true;
		KeyCode    m_toggleEnable;
		Connection m_inputConnection;
	};

} // namespace vzt

#endif // VATERAN_FREEFLY_HPP
