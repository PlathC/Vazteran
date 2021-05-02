
#ifndef VAZTERAN_IMAGE_HPP
#define VAZTERAN_IMAGE_HPP

#include <filesystem>
namespace fs = std::filesystem;

#include <glm/glm.hpp>

namespace vzt {
    class Image {
    public:
        Image(const fs::path& imagePath);
        Image(std::vector<uint8_t> imageData, uint32_t width, uint32_t height, uint8_t channels);
        Image(glm::vec4 color);
        Image() = default;

        Image& operator=(const fs::path& imagePath);
        Image& operator=(glm::vec4 color);

        const std::vector<uint8_t>& Data() { return m_data; }
        uint32_t Width() { return m_width; }
        uint32_t Height() { return m_height; }
        uint8_t Channels() { return m_channels; }
    private:
        std::vector<uint8_t> m_data;
        uint32_t m_width;
        uint32_t m_height;
        uint8_t m_channels;
    };
}


#endif //VAZTERAN_IMAGE_HPP
