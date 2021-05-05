#ifndef VAZTERAN_UTILS_HPP
#define VAZTERAN_UTILS_HPP

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace vzt {
    struct Size2D {
        uint32_t width;
        uint32_t height;
    };

    std::vector<char> ReadFile(const fs::path& filename);
}

#endif //VAZTERAN_UTILS_HPP
