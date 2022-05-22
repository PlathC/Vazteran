#ifndef VAZTERAN_TRANSFORM_HPP
#define VAZTERAN_TRANSFORM_HPP

#include "Vazteran/Core/Type.hpp"
#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct Transform
	{
		static constexpr bool in_place_delete = true;

		Mat4 get() const;

		void rotate(const Vec3& axis, float radians);

		Vec3 position;
		Quat rotation;

		const Transform* parent;
	};
} // namespace vzt

#endif // VAZTERAN_TRANSFORM_HPP
