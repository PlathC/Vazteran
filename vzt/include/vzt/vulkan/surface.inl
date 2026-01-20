#include "vzt/vulkan/surface.hpp"

namespace vzt
{
    inline const VkSurfaceKHR& Surface::getHandle() const { return m_handle; }
} // namespace vzt
