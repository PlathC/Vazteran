#ifndef VAZTERAN_TYPE_HPP
#define VAZTERAN_TYPE_HPP

#include <functional>
#include <type_traits>

namespace vzt
{
	template <class Enum>
	constexpr typename std::underlying_type<Enum>::type toUnderlying(const Enum e) noexcept
	{
		return static_cast<typename std::underlying_type<Enum>::type>(e);
	}

	template <class T>
	inline void combineHash(std::size_t& s, const T& v)
	{
		// HashCombine https://stackoverflow.com/a/19195373
		// Magic Numbers https://stackoverflow.com/a/4948967
		std::hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}

	template <typename Type>
	struct Size2D
	{
		Type width;
		Type height;
	};

	template <class Type>
	using Ptr = Type*;

	template <class Type>
	using Optional = std::reference_wrapper<Type>;

} // namespace vzt

#endif // VAZTERAN_TYPE_HPP
