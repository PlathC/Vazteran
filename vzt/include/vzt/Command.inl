#include "vzt/Command.hpp"

namespace vzt
{
    inline VkCommandBuffer& CommandBuffer::getHandle() { return *m_handle; }
} // namespace vzt
