
#ifndef VAZTERAN_MATH_HPP
#define VAZTERAN_MATH_HPP

#include <limits>

namespace vzt
{
	constexpr float Pi = 3.14159265359f;

	template <class T> constexpr T Max = std::numeric_limits<T>::max();
	template <class T> constexpr T Min = std::numeric_limits<T>::min();
	template <class T> constexpr T Epsilon = std::numeric_limits<T>::epsilon();
} // namespace vzt

#include "Vazteran/Core/Math/Aabb.hpp"
#include "Vazteran/Core/Math/Random.hpp"

#endif // VAZTERAN_MATH_HPP
