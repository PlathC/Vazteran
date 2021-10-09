#include "Vazteran/Core/Utils.hpp"

namespace vzt {
    std::vector<char> ReadFile(const fs::path& filename) {
        auto file = std::ifstream(filename.c_str(), std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file!");
        }

        std::size_t fileSize = static_cast<std::size_t>(file.tellg());
        auto buffer = std::vector<char>(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
}
