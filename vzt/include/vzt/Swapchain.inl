#include "vzt/Swapchain.hpp"

namespace vzt
{
    inline const std::vector<VkImage>& Swapchain::getImages() const { return m_images; }
} // namespace vzt
