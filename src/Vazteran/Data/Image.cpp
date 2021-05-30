#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <utility>

#include "Vazteran/Data/Image.hpp"

namespace vzt {
    Image::Image():
        Image({255, 255, 255, 255}, 1, 1, 4)  {
    }

    Image::Image(const fs::path &imagePath) {
        int width, height, channels;
        stbi_uc* pixels = stbi_load(imagePath.string().c_str(),
                                    &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }
        m_data = std::vector<uint8_t>(pixels, pixels + (width * height * 4));
        m_width = width;
        m_height = height;

        // We love the image with STBI_rgb_alpha, so we can set it as 4
        // even though channels might be different.
        m_channels = 4;
    }

    Image::Image(std::vector<uint8_t> imageData, uint32_t width, uint32_t height, uint8_t channels):
            m_data(std::move(imageData)), m_width(width), m_height(height), m_channels(channels) {
    }

    Image::Image(glm::vec4 color):
        Image({
             static_cast<uint8_t>(color.r * 255),
             static_cast<uint8_t>(color.g * 255),
             static_cast<uint8_t>(color.b * 255),
             static_cast<uint8_t>(color.a * 255)
        }, 1, 1, 4) {
    }

    Image& Image::operator=(const fs::path& imagePath) {
        int width, height, channels;
        stbi_uc* pixels = stbi_load(imagePath.string().c_str(),
                                    &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }
        m_data = std::vector<uint8_t>(pixels, pixels + (width * height * 4));
        m_width = width;
        m_height = height;
        m_channels = static_cast<uint8_t>(channels);

        return *this;
    }

    Image& Image::operator=(glm::vec4 color) {
        m_data = {
                static_cast<uint8_t>(color.r * 255),
                static_cast<uint8_t>(color.g * 255),
                static_cast<uint8_t>(color.b * 255),
                static_cast<uint8_t>(color.a * 255)
        };
        m_width = 1;
        m_height = 1;
        m_channels = 4;

        return *this;
    }

}