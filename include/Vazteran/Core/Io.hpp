#ifndef VAZTERAN_IO_HPP
#define VAZTERAN_IO_HPP

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace vzt
{
	std::vector<char> readFile(const fs::path& filename);
} // namespace vzt

#endif // VAZTERAN_IO_HPP
