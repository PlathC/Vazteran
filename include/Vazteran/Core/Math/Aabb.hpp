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
		AABB(const std::vector<vzt::Vec3>& vertices);
		AABB(const std::vector<vzt::AABB>& aabbs);

		void extend(const AABB& other);
		void refresh();

		const glm::vec3&                minimum() const { return m_minimum; }
		const glm::vec3&                maximum() const { return m_maximum; }
		const std::array<vzt::Vec3, 8>& getCVertices() const { return m_vertices; }
		std::array<vzt::Vec3, 8>&       getVertices() { return m_vertices; }

	  private:
		std::array<vzt::Vec3, 8> m_vertices;
		glm::vec3                m_minimum = vzt::Vec3(vzt::typeMax<float>);
		glm::vec3                m_maximum = vzt::Vec3(vzt::typeMin<float>);
	};
} // namespace vzt

#endif // VAZTERAN_AABB_HPP
