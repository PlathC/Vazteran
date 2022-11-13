#include "vzt/Vulkan/Swapchain.hpp"

namespace vzt
{
    inline void Swapchain::setExtent(Extent2D extent)
    {
        m_extent             = std::move(extent);
        m_framebufferResized = true;
    };
    inline View<Image> Swapchain::getImage(std::size_t i) const { return m_userImages[i]; }
    inline uint32_t    Swapchain::getImageNb() const { return m_imageNb; }
} // namespace vzt
