#ifndef VAZTERAN_UTILS_HPP
#define VAZTERAN_UTILS_HPP

#include <filesystem>
#include <fstream>

#include <random>

namespace fs = std::filesystem;

namespace vzt {
    std::vector<char> ReadFile(const fs::path& filename);
}

#endif //VAZTERAN_UTILS_HPP
