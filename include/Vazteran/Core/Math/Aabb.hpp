#ifndef VAZTERAN_AABB_HPP
#define VAZTERAN_AABB_HPP

#include <array>
#include <vector>

#include "Vazteran/Core/Math.hpp"

namespace vzt
{
	class AABB
	{
	  public:
		AABB() = default;
		AABB(const std::vector<vzt::Vec3> &vertices);
		AABB(const std::vector<vzt::AABB> &aabbs);

		void Extend(const AABB &other);
		void Refresh();

		const glm::vec3 &Min() const
		{
			return m_minimum;
		}
		const glm::vec3 &Max() const
		{
			return m_maximum;
		}
		const std::array<vzt::Vec3, 8> &CVertices() const
		{
			return m_vertices;
		}
		std::array<vzt::Vec3, 8> &Vertices()
		{
			return m_vertices;
		}

	  private:
		std::array<vzt::Vec3, 8> m_vertices;
		glm::vec3 m_minimum = vzt::Vec3(vzt::Max<float>);
		glm::vec3 m_maximum = vzt::Vec3(vzt::Min<float>);
	};
} // namespace vzt

#endif // VAZTERAN_AABB_HPP
