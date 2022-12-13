#include "vzt/Vulkan/Raytracing/AccelerationStructure.hpp"

namespace vzt
{
    inline uint64_t                   BottomGeometryAs::getSize() const { return m_size; }
    inline View<Buffer>               BottomGeometryAs::getBuffer() const { return m_buffer; }
    inline VkAccelerationStructureKHR BottomGeometryAs::getHandle() const { return m_handle; }
} // namespace vzt
