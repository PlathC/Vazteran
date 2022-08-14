#include "Vazteran/Data/Transform.hpp"
#include <glm/gtc/quaternion.hpp>

namespace vzt
{
	Mat4 Transform::get() const
	{
		Mat4 transform = glm::mat4_cast(rotation);
		transform[3]   = Vec4(position, 1.f);

		return transform;
	}

	void Transform::set(const Mat4& value)
	{
		rotation = glm::quat_cast(value);
		position = value[3];
	}

	void Transform::translate(const Vec3& translation) { position += translation; }
	void Transform::translateRelative(Vec3 translation) { position += rotation * translation; }

	void Transform::rotate(const Vec3& axis, const float radians)
	{
		rotation = glm::normalize(rotation * glm::angleAxis(radians, axis));
	}
	void Transform::rotate(const Quat& appliedRotation) { rotation = glm::normalize(rotation * appliedRotation); }
	void Transform::rotateAround(const Vec3& pivot, const Vec3& axis, const float radians)
	{
		// Based on https://stackoverflow.com/a/49824672
		Vec3        targetDirection = pivot - position;
		const float distance        = glm::length(targetDirection);
		if (distance < 1e-6f)
		{
			rotation = Quat{1.f, 0.f, 0.f, 0.f};
			return;
		}

		targetDirection /= distance;

		constexpr Vec3 up = {0.f, 0.f, 1.f};
		if (glm::dot(targetDirection, up) > 1.f - 1e-6f)
			rotation = glm::quatLookAt(targetDirection, Vec3{0.f, 1.f, 0.f});
		else
			rotation = glm::quatLookAt(targetDirection, up);
	}

} // namespace vzt
