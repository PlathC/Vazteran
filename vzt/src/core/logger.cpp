#include "vzt/core/logger.hpp"

#include <chrono>
#include <ctime>

#include <fmt/chrono.h>

namespace vzt::logger
{
    static std::string_view toString(Level level)
    {
        if (level == Level::Info)
            return "Info";
        else if (level == Level::Warning)
            return "Warning";
        else if (level == Level::Debug)
            return "Debug";
        else if (level == Level::Error)
            return "Error";

        return "";
    }

    void log(const Level level, std::string_view str)
    {
        const std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm           local;

#ifdef WIN32
        ::localtime_s(&local, &currentTime);
#else
        ::localtime_r(&currentTime, &local);
#endif
        fmt::print("[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n", local, toString(level), str);
    }

} // namespace vzt::logger
