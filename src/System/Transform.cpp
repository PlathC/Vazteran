#include "Vazteran/System/Transform.hpp"

#include <glm/gtx/quaternion.hpp>

namespace vzt
{
	Mat4 Transform::get() const
	{
		Mat4 transform = glm::toMat4(rotation);
		glm::translate(transform, position);

		if (parent)
			transform = parent->get() * transform;
		return transform;
	}

	void Transform::rotate(const Vec3& axis, float radians) { rotation = glm::angleAxis(radians, axis) * rotation; }

} // namespace vzt
