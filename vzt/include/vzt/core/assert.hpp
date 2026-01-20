#ifndef VZT_ASSERT_HPP
#define VZT_ASSERT_HPP

#include <sstream>
#include <string_view>

#include "vzt/core/logger.hpp"

#define VZT_ASSERT(x)                                     \
    {                                                     \
        if (!(x))                                         \
        {                                                 \
            vzt::assertionFailed(#x, __FILE__, __LINE__); \
        }                                                 \
    }

#define VZT_NOT_REACHED()                    \
    {                                        \
        vzt::notReached(__FILE__, __LINE__); \
    }

namespace vzt
{
    // https://github.com/alicevision/geogram/blob/dfc40f6805e962274665792932fc736d350c80b8/src/lib/geogram/basic/assert.cpp#L106C1-L130C1
    inline void assertionFailed(std::string_view condition_string, std::string_view file, int line)
    {
        vzt::logger::error("Assertion failed: {}\nFile: {}\nLine: {}", condition_string, file, line);
        std::abort();
    }

    inline void notReached(std::string_view file, int line)
    {
        vzt::logger::error("Should not have reached! File: {}\nLine: {}", file, line);
        std::abort();
    }
} // namespace vzt

#endif // VZT_ASSERT_HPP