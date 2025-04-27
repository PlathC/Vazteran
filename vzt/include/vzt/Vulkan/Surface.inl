#include "vzt/Vulkan/Surface.hpp"

namespace vzt
{
    inline const VkSurfaceKHR& Surface::getHandle() const { return m_handle; }
} // namespace vzt
