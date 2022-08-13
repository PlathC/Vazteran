#ifndef VAZTERAN_META_HPP
#define VAZTERAN_META_HPP

#include "Vazteran/Core/Type.hpp"

#define BITWISE_FUNCTION(Type)                                                 \
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
	inline constexpr Type operator~(const Type m) { return static_cast<Type>(~vzt::toUnderlying(m)); }

#define TO_VULKAN_FUNCTION(BaseType, VulkanType) \
	inline constexpr VulkanType toVulkan(const BaseType l) { return static_cast<VulkanType>(l); }

namespace vzt
{
	// Based on https://stackoverflow.com/a/39348287
	namespace internal
	{
		template <class X, class Y, class Op>
		struct ValidOperationImpl
		{
			template <class U, class L, class R>
			static auto test(int)
			    -> decltype(std::declval<U>()(std::declval<L>(), std::declval<R>()), void(), std::true_type());

			template <class U, class L, class R>
			static auto test(...) -> std::false_type;

			using type = decltype(test<Op, X, Y>(0));
		};

	} // namespace internal
	template <class X, class Y, class Op>
	using hasOperator = typename internal::ValidOperationImpl<X, Y, Op>::type;

} // namespace vzt

#endif // VAZTERAN_META_HPP
