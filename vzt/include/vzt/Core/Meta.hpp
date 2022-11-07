#ifndef VZT_META_HPP
#define VZT_META_HPP

#include <type_traits>

namespace vzt
{
    template <class Enum>
    constexpr typename std::underlying_type<Enum>::type toUnderlying(const Enum e) noexcept
    {
        return static_cast<typename std::underlying_type<Enum>::type>(e);
    }
} // namespace vzt

#define VZT_DEFINE_BITWISE_FUNCTIONS(Type)                                     \
    inline constexpr Type operator&(const Type l, const Type r)                \
    {                                                                          \
        return static_cast<Type>(vzt::toUnderlying(l) & vzt::toUnderlying(r)); \
    }                                                                          \
                                                                               \
    inline constexpr Type operator|(const Type l, const Type r)                \
    {                                                                          \
        return static_cast<Type>(vzt::toUnderlying(l) | vzt::toUnderlying(r)); \
    }                                                                          \
                                                                               \
    inline constexpr Type operator~(const Type m)                              \
    {                                                                          \
        return static_cast<Type>(~vzt::toUnderlying(m));                       \
    }                                                                          \
                                                                               \
    inline constexpr bool any(const Type m)                                    \
    {                                                                          \
        return vzt::toUnderlying(m) != 0;                                      \
    }

#endif // VZT_META_HPP
