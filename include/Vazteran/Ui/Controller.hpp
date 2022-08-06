#ifndef VATERAN_CONTROLLER_HPP
#define VATERAN_CONTROLLER_HPP

#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct AABB;
	struct Camera;
	struct Transform;

	void      updateFirstPerson(const Vec2& deltaCursorPosition, Transform& transform);
	Transform fromAabb(const Camera& camera, const AABB& reference);

	class FirstPersonController
	{
	  public:
		FirstPersonController(Transform* transform);
		~FirstPersonController() = default;

		void operator()(const Vec2& deltaCursorPosition);

	  private:
		bool       m_enabled   = false;
		Transform* m_transform = nullptr;
	};
} // namespace vzt

#endif // VATERAN_CONTROLLER_HPP
