// Based on Eigen handling of disabled warnings
// See: https://gitlab.com/libeigen/eigen/-/blob/master/Eigen/src/Core/util/DisableStupidWarnings.h?ref_type=heads
#ifndef VZT_CORE_DISABLE_WARNINGS
#define VZT_CORE_DISABLE_WARNINGS

#if defined(_MSC_VER)
#pragma warning(push)
// #pragma warning( disable : <> )

#elif defined(__INTEL_COMPILER)
#pragma warning push

#elif defined(__clang__)
#pragma clang diagnostic push
#if defined(__has_warning)
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"
#endif
#if __has_warning("-Wunused-parameter")
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#if __has_warning("-Wgnu-anonymous-struct")
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#endif
#if __has_warning("-Wnested-anon-types")
#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif
#if __has_warning("-Wdeprecated-declarations")
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#if __has_warning("-Wsign-compare")
#pragma clang diagnostic ignored "-Wsign-compare"
#endif
#if __has_warning("-Wmissing-braces")
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif
#if __has_warning("-Wformat")
#pragma clang diagnostic ignored "-Wformat"
#endif
#if __has_warning("-Wnon-virtual-dtor")
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#if __has_warning("-Wold-style-cast")
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#if __has_warning("-Wsign-conversion")
#pragma clang diagnostic ignored "-Wsign-conversion"
#endif
#endif

#elif defined(__GNUC__)
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wnull-dereference"

#endif

#endif // VZT_CORE_DISABLE_WARNINGS
