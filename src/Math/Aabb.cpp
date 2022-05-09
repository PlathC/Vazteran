#include "Vazteran/Math/Aabb.hpp"

namespace vzt
{
	AABB::AABB(const std::vector<vzt::Vec3>& vertices)
	{
		assert(!vertices.empty() && "Vertices array should not be empty");

		m_minimum = m_maximum = vertices[0];
		for (const auto& vertex : vertices)
		{
			if (vertex.x < m_minimum.x)
			{
				m_minimum.x = vertex.x;
			}

			if (vertex.y < m_minimum.y)
			{
				m_minimum.y = vertex.y;
			}

			if (vertex.z < m_minimum.z)
			{
				m_minimum.z = vertex.z;
			}

			if (vertex.x > m_maximum.x)
			{
				m_maximum.x = vertex.x;
			}

			if (vertex.y > m_maximum.y)
			{
				m_maximum.y = vertex.y;
			}

			if (vertex.z > m_maximum.z)
			{
				m_maximum.z = vertex.z;
			}
		}

		m_vertices = std::array<glm::vec3, 8>{
		    // Bottom
		    vzt::Vec3{m_minimum.x, m_minimum.y, m_minimum.z},
		    vzt::Vec3{m_maximum.x, m_minimum.y, m_minimum.z},
		    vzt::Vec3{m_minimum.x, m_maximum.y, m_minimum.z},
		    vzt::Vec3{m_maximum.x, m_maximum.y, m_minimum.z},

		    // Top
		    vzt::Vec3{m_minimum.x, m_minimum.y, m_maximum.z},
		    vzt::Vec3{m_maximum.x, m_minimum.y, m_maximum.z},
		    vzt::Vec3{m_minimum.x, m_maximum.y, m_maximum.z},
		    vzt::Vec3{m_maximum.x, m_maximum.y, m_maximum.z},
		};
	}

	AABB::AABB(const std::vector<vzt::AABB>& aabbs)
	{
		assert(!aabbs.empty() && "Vertices array should not be empty");

		m_minimum = aabbs[0].m_minimum;
		m_maximum = aabbs[0].m_maximum;
		for (const auto& aabb : aabbs)
		{
			m_minimum.x = std::min(aabb.m_minimum.x, m_minimum.x);
			m_minimum.y = std::min(aabb.m_minimum.y, m_minimum.y);
			m_minimum.z = std::min(aabb.m_minimum.z, m_minimum.z);

			m_maximum.x = std::max(aabb.m_maximum.x, m_maximum.x);
			m_maximum.y = std::max(aabb.m_maximum.y, m_maximum.y);
			m_maximum.z = std::max(aabb.m_maximum.z, m_maximum.z);
		}

		m_vertices = std::array<vzt::Vec3, 8>{
		    // Bottom
		    vzt::Vec3{m_minimum.x, m_minimum.y, m_minimum.z},
		    vzt::Vec3{m_maximum.x, m_minimum.y, m_minimum.z},
		    vzt::Vec3{m_minimum.x, m_maximum.y, m_minimum.z},
		    vzt::Vec3{m_maximum.x, m_maximum.y, m_minimum.z},

		    // Top
		    vzt::Vec3{m_minimum.x, m_minimum.y, m_maximum.z},
		    vzt::Vec3{m_maximum.x, m_minimum.y, m_maximum.z},
		    vzt::Vec3{m_minimum.x, m_maximum.y, m_maximum.z},
		    vzt::Vec3{m_maximum.x, m_maximum.y, m_maximum.z},
		};
	}

	void AABB::extend(const AABB& other)
	{
		m_minimum.x = std::min(other.m_minimum.x, m_minimum.x);
		m_minimum.y = std::min(other.m_minimum.y, m_minimum.y);
		m_minimum.z = std::min(other.m_minimum.z, m_minimum.z);

		m_maximum.x = std::max(other.m_maximum.x, m_maximum.x);
		m_maximum.y = std::max(other.m_maximum.y, m_maximum.y);
		m_maximum.z = std::max(other.m_maximum.z, m_maximum.z);

		m_vertices = std::array<glm::vec3, 8>{
		    // Bottom
		    vzt::Vec3{m_minimum.x, m_minimum.y, m_minimum.z},
		    vzt::Vec3{m_maximum.x, m_minimum.y, m_minimum.z},
		    vzt::Vec3{m_minimum.x, m_maximum.y, m_minimum.z},
		    vzt::Vec3{m_maximum.x, m_maximum.y, m_minimum.z},

		    // Top
		    vzt::Vec3{m_minimum.x, m_minimum.y, m_maximum.z},
		    vzt::Vec3{m_maximum.x, m_minimum.y, m_maximum.z},
		    vzt::Vec3{m_minimum.x, m_maximum.y, m_maximum.z},
		    vzt::Vec3{m_maximum.x, m_maximum.y, m_maximum.z},
		};
	}

	void AABB::refresh()
	{
		m_minimum = m_maximum = m_vertices[0];
		for (const auto& vertex : m_vertices)
		{
			if (vertex.x < m_minimum.x)
			{
				m_minimum.x = vertex.x;
			}

			if (vertex.y < m_minimum.y)
			{
				m_minimum.y = vertex.y;
			}

			if (vertex.z < m_minimum.z)
			{
				m_minimum.z = vertex.z;
			}

			if (vertex.x > m_maximum.x)
			{
				m_maximum.x = vertex.x;
			}

			if (vertex.y > m_maximum.y)
			{
				m_maximum.y = vertex.y;
			}

			if (vertex.z > m_maximum.z)
			{
				m_maximum.z = vertex.z;
			}
		}
	}
} // namespace vzt
