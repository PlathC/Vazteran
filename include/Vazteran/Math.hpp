
#ifndef VAZTERAN_MATH_HPP
#define VAZTERAN_MATH_HPP

namespace vzt {
    template <class T>
    inline void HashCombine(std::size_t& s, const T& v)
    {
        // HashCombine https://stackoverflow.com/a/19195373
        // Magic Numbers https://stackoverflow.com/a/4948967
        std::hash<T> h;
        s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
    }

}

#endif //VAZTERAN_MATH_HPP
