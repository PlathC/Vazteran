#ifndef VAZTERAN_IO_HPP
#define VAZTERAN_IO_HPP

#include <filesystem>
#include <vector>

namespace vzt
{
	using Path = std::filesystem::path;

	std::string readFile(const Path& path);
} // namespace vzt

#endif // VAZTERAN_IO_HPP
