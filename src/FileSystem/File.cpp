#include <fstream>

#include "Vazteran/FileSystem/File.hpp"

namespace vzt
{
	std::vector<char> readFile(const Path& path)
	{
		auto file = std::ifstream(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file!");
		}

		std::size_t fileSize = static_cast<std::size_t>(file.tellg());
		auto        buffer   = std::vector<char>(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
} // namespace vzt
