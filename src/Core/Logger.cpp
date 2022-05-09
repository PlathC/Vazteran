#include "Vazteran/Core/Logger.hpp"

namespace vzt
{
	Logger::Logger() { spdlog::set_pattern("[%H:%M:%S %z] [%^---%L---%$] %v"); }

	const Logger& Logger::get()
	{
		static Logger instance;
		return instance;
	}
} // namespace vzt
