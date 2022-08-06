#ifndef VAZTERAN_AABB_HPP
#define VAZTERAN_AABB_HPP

#include <array>
#include <vector>

#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	struct AABB
	{
		std::array<Vec3, 8> vertices;
		Vec3                minimum = Vec3(vzt::typeMax<float>);
		Vec3                maximum = Vec3(vzt::typeMin<float>);

		AABB() = default;
		AABB(const std::vector<Vec3>& vertices);
		AABB(const std::vector<AABB>& aabbs);

		void extend(const AABB& other);
		void refresh();
	};
} // namespace vzt

#endif // VAZTERAN_AABB_HPP
