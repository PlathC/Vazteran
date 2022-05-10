#ifndef VAZTERAN_LOGGER_HPP
#define VAZTERAN_LOGGER_HPP

#include <spdlog/spdlog.h>

namespace vzt
{
	class Logger
	{
	  public:
		enum class LogLevel
		{
			Info,
			Debug,
			Warning,
			Error
		};

		~Logger() = default;

		static const Logger& get();

		template <typename... T>
		void log(LogLevel logLevel, fmt::format_string<T...> fmt, T&&... args) const
		{
			switch (logLevel)
			{
			case LogLevel::Info: spdlog::info(fmt, args...); break;
			case LogLevel::Debug: spdlog::debug(fmt, args...); break;
			case LogLevel::Warning: spdlog::warn(fmt, args...); break;
			case LogLevel::Error: spdlog::error(fmt, args...); break;
			}
		}

	  private:
		Logger();
	};
	template <typename... T>
	inline void VZT_INFO(fmt::format_string<T...> fmt, T&&... args)
	{
		Logger::get().log(Logger::LogLevel::Info, fmt, args...);
	}
	template <typename... T>
	inline void VZT_DEBUG(fmt::format_string<T...> fmt, T&&... args)
	{
		Logger::get().log(Logger::LogLevel::Debug, fmt, args...);
	}
	template <typename... T>
	inline void VZT_WARNING(fmt::format_string<T...> fmt, T&&... args)
	{
		Logger::get().log(Logger::LogLevel::Warning, fmt, args...);
	}
	template <typename... T>
	inline void VZT_ERROR(fmt::format_string<T...> fmt, T&&... args)
	{
		Logger::get().log(Logger::LogLevel::Error, fmt, args...);
	}
} // namespace vzt

#endif // VAZTERAN_LOGGER_HPP
