
#ifndef VAZTERAN_MATH_HPP
#define VAZTERAN_MATH_HPP

#include <limits>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

namespace vzt
{
	constexpr float Pi = 3.14159265359f;

	template <class T>
	constexpr T Max = std::numeric_limits<T>::max();
	template <class T>
	constexpr T Min = std::numeric_limits<T>::min();
	template <class T>
	constexpr T Epsilon = std::numeric_limits<T>::epsilon();

	using Vec2  = glm::vec2;
	using Ivec2 = glm::ivec2;
	using Dvec2 = glm::dvec2;
	using Vec3  = glm::vec3;
	using Ivec3 = glm::ivec3;
	using Dvec3 = glm::dvec3;
	using Vec4  = glm::vec4;
	using Ivec4 = glm::ivec4;
	using Dvec4 = glm::dvec4;

	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;

	template <typename Type>
	struct Size2D
	{
		Type width;
		Type height;
	};

	constexpr float ToRadians(float degrees) { return degrees * vzt::Pi / 180.f; }
	constexpr float ToDegrees(float radians) { return radians * 180.f / vzt::Pi; }
} // namespace vzt

#include "Vazteran/Core/Math/Aabb.hpp"
#include "Vazteran/Core/Math/Random.hpp"

#endif // VAZTERAN_MATH_HPP
