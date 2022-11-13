#include "vzt/Core/Logger.hpp"

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
        ::localtime_s(&local, &currentTime);

        fmt::print("[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n", local, toString(level), str);
    }

} // namespace vzt::logger
