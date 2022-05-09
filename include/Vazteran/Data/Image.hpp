
#ifndef VAZTERAN_IMAGE_HPP
#define VAZTERAN_IMAGE_HPP

#include <filesystem>
namespace fs = std::filesystem;

#include "Vazteran/Math/Math.hpp"

namespace vzt
{
	class Image
	{
	  public:
		Image();
		Image(const fs::path& imagePath);
		Image(std::vector<uint8_t> imageData, uint32_t width, uint32_t height, uint8_t channels);
		Image(glm::vec4 color);

		Image& operator=(const fs::path& imagePath);
		Image& operator=(glm::vec4 color);

		const std::vector<uint8_t>& data() { return m_data; }
		uint32_t                    width() const { return m_width; }
		uint32_t                    height() const { return m_height; }
		uint8_t                     channels() const { return m_channels; }

	  private:
		std::vector<uint8_t> m_data;
		uint32_t             m_width;
		uint32_t             m_height;
		uint8_t              m_channels;
	};
} // namespace vzt

#endif // VAZTERAN_IMAGE_HPP
