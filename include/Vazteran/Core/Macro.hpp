#ifndef VAZTERAN_CORE_MACRO_HPP
#define VAZTERAN_CORE_MACRO_HPP

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
	inline constexpr VulkanType ToVulkan(const BaseType l) { return static_cast<VulkanType>(l); }

#endif // VAZTERAN_CORE_MACRO_HPP
