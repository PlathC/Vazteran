#include "vzt/Core/File.hpp"

#include <fstream>

#include "vzt/Core/Logger.hpp"

namespace vzt
{
    std::string readFile(const Path& path)
    {
        std::ifstream file{path, std::ios::ate | std::ios::binary};
        if (!file.is_open())
        {
            logger::error("Failed to open file {} !", path.string());
            return "";
        }

        const std::size_t fileSize = static_cast<std::size_t>(file.tellg());
        std::string       buffer{};
        buffer.resize(fileSize);

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

        file.close();

        return buffer;
    }
} // namespace vzt
