#include "vzt/Vulkan/Command.hpp"

namespace vzt
{
    inline VkCommandBuffer CommandBuffer::getHandle() const { return m_handle; }
} // namespace vzt
