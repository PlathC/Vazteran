#include "Vazteran/Core/Logger.hpp"

namespace vzt
{
	Logger::Logger()
	{
		spdlog::set_pattern("[%H:%M:%S %z] [%^---%L---%$] %v");
#ifndef NDEBUG
		spdlog::set_level(spdlog::level::debug);
#endif // NDEBUG
	}

	const Logger& Logger::get()
	{
		static Logger instance;

		return instance;
	}
} // namespace vzt
