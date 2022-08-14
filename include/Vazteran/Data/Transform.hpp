#ifndef VAZTERAN_TRANSFORM_HPP
#define VAZTERAN_TRANSFORM_HPP

#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct Transform
	{
		Vec3 position = Vec3(0.f);
		Quat rotation = Quat{0.f, 0.f, 0.f, 1.f};

		Mat4 get() const;
		void set(const Mat4& value);

		void lookAt(const Vec3& target);

		void translate(const Vec3& translation);
		void translateRelative(Vec3 translation);

		void rotate(const Vec3& axis, const float radians);
		void rotate(const Quat& appliedRotation);
		void rotateAround(const Vec3& pivot, const Vec3& axis, const float radians);
	};
} // namespace vzt

#endif // VAZTERAN_TRANSFORM_HPP
