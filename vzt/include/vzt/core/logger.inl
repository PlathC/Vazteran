#include "vzt/core/logger.hpp"

namespace vzt::logger
{
    template <typename... T>
    void log(Level level, std::string_view str, T&&... args)
    {
        log(level, fmt::format(fmt::runtime(str), std::forward<T>(args)...));
    }

    template <typename... T>
    void info(std::string_view str, T&&... args)
    {
        log(Level::Info, str, std::forward<T>(args)...);
    }

    template <typename... T>
    void debug([[maybe_unused]] std::string_view str, [[maybe_unused]] T&&... args)
    {
#ifndef NDEBUG
        log(Level::Debug, str, std::forward<T>(args)...);
#endif // NDEBUG
    }

    template <typename... T>
    void warn(std::string_view str, T&&... args)
    {
        log(Level::Warning, str, std::forward<T>(args)...);
    }

    template <typename... T>
    void error(std::string_view str, T&&... args)
    {
        log(Level::Error, str, std::forward<T>(args)...);
    }
} // namespace vzt::logger
