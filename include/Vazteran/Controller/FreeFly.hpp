#ifndef VATERAN_FREEFLY_HPP
#define VATERAN_FREEFLY_HPP

#include "Vazteran/Controller/CameraController.hpp"
#include "Vazteran/Core/Event.hpp"
#include "Vazteran/Math/Math.hpp"
#include "Vazteran/Ui/Inputs.hpp"

namespace vzt
{
	struct AABB;
	struct Camera;
	struct Transform;
	class Window;

	class FreeFly : public CameraController
	{
	  public:
		FreeFly(Window& window, Camera& camera, Transform& transform, KeyCode toggleEnable = KeyCode::Tab);
		~FreeFly() = default;

		void setEnable(bool enable) { m_enabled = enable; }
		bool getEnable() const;
		void operator()(const Inputs& inputs);

	  private:
		bool    m_enabled = true;
		KeyCode m_toggleEnable;

		Connection m_inputConnection;
		Transform* m_transform;
	};

} // namespace vzt

#endif // VATERAN_FREEFLY_HPP
