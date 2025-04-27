#include "vzt/Vulkan/Swapchain.hpp"

namespace vzt
{
    inline void              Swapchain::recreate() { m_framebufferResized = true; };
    inline Extent2D          Swapchain::getExtent() const { return m_extent; }
    inline View<DeviceImage> Swapchain::getImage(std::size_t i) const { return m_userImages[i]; }
    inline uint32_t          Swapchain::getImageNb() const { return m_imageNb; }
    inline Format            Swapchain::getFormat() const { return m_format; }
} // namespace vzt
