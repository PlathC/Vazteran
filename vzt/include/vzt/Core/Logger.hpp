#ifndef VZT_CORE_LOGGER_HPP
#define VZT_CORE_LOGGER_HPP

#include <fmt/core.h>
#include <fmt/format.h>

namespace vzt::logger
{
    enum class Level
    {
        Info,
        Debug,
        Warning,
        Error
    };

    void log(Level, std::string_view str);

    template <typename... T>
    void log(std::string_view str, T&&... args);

    template <typename... T>
    void info(std::string_view str, T&&... args);

    template <typename... T>
    void debug(std::string_view str, T&&... args);

    template <typename... T>
    void warn(std::string_view str, T&&... args);

    template <typename... T>
    void error(std::string_view str, T&&... args);
} // namespace vzt::logger

#include "vzt/Core/Logger.inl"

#endif // VZT_CORE_LOGGER_HPP
