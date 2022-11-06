#include "vzt/Core/Logger.hpp"

namespace vzt::logger
{
    template <typename... T>
    void log(Level level, std::string_view str, T&&... args)
    {
        log(level, fmt::format(str, args...));
    }

    template <typename... T>
    void info(std::string_view str, T&&... args)
    {
        log(Level::Info, str, std::forward<T>(args)...);
    }

    template <typename... T>
    void debug(std::string_view str, T&&... args)
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
