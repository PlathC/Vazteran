#ifndef VAZTERAN_CORE_RANDOM_HPP
#define VAZTERAN_CORE_RANDOM_HPP

namespace vzt
{

	template <class T> inline T Random(const T min, const T max)
	{
		static thread_local std::mt19937 generator;

		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}
		else if (std::is_floating_point_v<T>)
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}
} // namespace vzt

#endif // VAZTERAN_CORE_RANDOM_HPP
