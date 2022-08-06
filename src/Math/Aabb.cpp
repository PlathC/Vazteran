#include "Vazteran/Math/Aabb.hpp"

namespace vzt
{
	AABB::AABB(const std::vector<vzt::Vec3>& input)
	{
		assert(!input.empty() && "Vertices array should not be empty");

		minimum = maximum = input[0];
		for (const auto& vertex : input)
		{
			if (vertex.x < minimum.x)
				minimum.x = vertex.x;

			if (vertex.y < minimum.y)
				minimum.y = vertex.y;

			if (vertex.z < minimum.z)
				minimum.z = vertex.z;

			if (vertex.x > maximum.x)
				maximum.x = vertex.x;

			if (vertex.y > maximum.y)
				maximum.y = vertex.y;

			if (vertex.z > maximum.z)
				maximum.z = vertex.z;
		}

		vertices = std::array<Vec3, 8>{
		    // Bottom
		    Vec3{minimum.x, minimum.y, minimum.z},
		    Vec3{maximum.x, minimum.y, minimum.z},
		    Vec3{minimum.x, maximum.y, minimum.z},
		    Vec3{maximum.x, maximum.y, minimum.z},

		    // Top
		    Vec3{minimum.x, minimum.y, maximum.z},
		    Vec3{maximum.x, minimum.y, maximum.z},
		    Vec3{minimum.x, maximum.y, maximum.z},
		    Vec3{maximum.x, maximum.y, maximum.z},
		};
	}

	AABB::AABB(const std::vector<vzt::AABB>& aabbs)
	{
		assert(!aabbs.empty() && "Vertices array should not be empty");

		minimum = aabbs[0].minimum;
		maximum = aabbs[0].maximum;
		for (const auto& aabb : aabbs)
		{
			minimum.x = std::min(aabb.minimum.x, minimum.x);
			minimum.y = std::min(aabb.minimum.y, minimum.y);
			minimum.z = std::min(aabb.minimum.z, minimum.z);

			maximum.x = std::max(aabb.maximum.x, maximum.x);
			maximum.y = std::max(aabb.maximum.y, maximum.y);
			maximum.z = std::max(aabb.maximum.z, maximum.z);
		}

		vertices = std::array<Vec3, 8>{
		    // Bottom
		    Vec3{minimum.x, minimum.y, minimum.z},
		    Vec3{maximum.x, minimum.y, minimum.z},
		    Vec3{minimum.x, maximum.y, minimum.z},
		    Vec3{maximum.x, maximum.y, minimum.z},

		    // Top
		    Vec3{minimum.x, minimum.y, maximum.z},
		    Vec3{maximum.x, minimum.y, maximum.z},
		    Vec3{minimum.x, maximum.y, maximum.z},
		    Vec3{maximum.x, maximum.y, maximum.z},
		};
	}

	void AABB::extend(const AABB& other)
	{
		minimum.x = std::min(other.minimum.x, minimum.x);
		minimum.y = std::min(other.minimum.y, minimum.y);
		minimum.z = std::min(other.minimum.z, minimum.z);

		maximum.x = std::max(other.maximum.x, maximum.x);
		maximum.y = std::max(other.maximum.y, maximum.y);
		maximum.z = std::max(other.maximum.z, maximum.z);

		vertices = std::array<Vec3, 8>{
		    // Bottom
		    Vec3{minimum.x, minimum.y, minimum.z},
		    Vec3{maximum.x, minimum.y, minimum.z},
		    Vec3{minimum.x, maximum.y, minimum.z},
		    Vec3{maximum.x, maximum.y, minimum.z},

		    // Top
		    Vec3{minimum.x, minimum.y, maximum.z},
		    Vec3{maximum.x, minimum.y, maximum.z},
		    Vec3{minimum.x, maximum.y, maximum.z},
		    Vec3{maximum.x, maximum.y, maximum.z},
		};
	}

	void AABB::refresh()
	{
		minimum = maximum = vertices[0];
		for (const auto& vertex : vertices)
		{
			if (vertex.x < minimum.x)
				minimum.x = vertex.x;

			if (vertex.y < minimum.y)
				minimum.y = vertex.y;

			if (vertex.z < minimum.z)
				minimum.z = vertex.z;

			if (vertex.x > maximum.x)
				maximum.x = vertex.x;

			if (vertex.y > maximum.y)
				maximum.y = vertex.y;

			if (vertex.z > maximum.z)
				maximum.z = vertex.z;
		}
	}
} // namespace vzt
