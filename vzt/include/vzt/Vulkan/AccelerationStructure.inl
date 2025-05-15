#include "vzt/Vulkan/AccelerationStructure.hpp"

namespace vzt
{
    inline AccelerationStructureType AccelerationStructure::getType() const { return m_type; }
    inline uint64_t                  AccelerationStructure::getSize() const { return m_size; }
    inline uint64_t                  AccelerationStructure::getScratchBufferSize() const { return m_scratchBufferSize; }
    inline View<Buffer>              AccelerationStructure::getBuffer() const { return m_buffer; }
    inline const std::vector<GeometryAccelerationStructureBuilder>& AccelerationStructure::getGeometries() const { return m_geometries; }
    inline uint64_t AccelerationStructure::getDeviceAddress() const { return m_deviceAddress; }
} // namespace vzt
