
#ifndef VAZTERAN_MATH_HPP
#define VAZTERAN_MATH_HPP

#include <cstddef>
#include <limits>

namespace vzt {
    const float Pi = 3.14159265359f;

    const float MaxFloat = std::numeric_limits<float>::max();
    const float MinFloat = std::numeric_limits<float>::min();

    template <class T>
    inline void HashCombine(std::size_t& s, const T& v)
    {
        // HashCombine https://stackoverflow.com/a/19195373
        // Magic Numbers https://stackoverflow.com/a/4948967
        std::hash<T> h;
        s^= h(v) + 0x9e3779b9 + (s << 6) + (s>> 2);
    }


    template<class T>
    inline T Random(const T min, const T max)
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
}

#endif //VAZTERAN_MATH_HPP
