#include "vzt/Swapchain.hpp"

namespace vzt
{
    inline const std::vector<VkImage>& Swapchain::getImages() const { return m_images; }
    inline uint32_t                    Swapchain::getImageNb() const { return m_imageNb; }
} // namespace vzt
