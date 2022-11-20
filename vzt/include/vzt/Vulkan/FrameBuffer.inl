#include "vzt/Vulkan/FrameBuffer.hpp"

namespace vzt
{
    inline VkFramebuffer FrameBuffer::getHandle() const { return m_handle; }
    inline Extent2D      FrameBuffer::getSize() const { return m_size; }

} // namespace vzt
