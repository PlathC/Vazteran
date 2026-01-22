// Based on Eigen handling of disabled warnings
// See: https://gitlab.com/libeigen/eigen/-/blob/master/Eigen/src/Core/util/DisableStupidWarnings.h?ref_type=heads
#ifdef VZT_CORE_ENABLE_WARNINGS
#undef VZT_CORE_ENABLE_WARNINGS

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined __INTEL_COMPILER
#pragma warning pop
#elif defined __clang__
#pragma clang diagnostic pop
#elif defined __GNUC__ && !defined(__FUJITSU) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif

#endif // VZT_CORE_ENABLE_WARNINGS
