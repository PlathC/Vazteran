#ifndef VAZTERAN_UTILS_HPP
#define VAZTERAN_UTILS_HPP

#include <filesystem>
#include <fstream>

#include <random>

namespace fs = std::filesystem;

namespace vzt
{
	std::vector<char> ReadFile(const fs::path &filename);

	template <class T> inline void HashCombine(std::size_t &s, const T &v)
	{
		// HashCombine https://stackoverflow.com/a/19195373
		// Magic Numbers https://stackoverflow.com/a/4948967
		std::hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}
} // namespace vzt

#endif // VAZTERAN_UTILS_HPP
