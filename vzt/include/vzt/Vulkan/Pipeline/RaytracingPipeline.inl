#include "vzt/Vulkan/Pipeline/RaytracingPipeline.hpp"

namespace vzt
{
    inline CSpan<uint8_t> RaytracingPipeline::getShaderHandleStorage() const { return m_shaderHandleStorage; }
    inline uint32_t       RaytracingPipeline::getShaderHandleSize() const { return m_handleSize; }
    inline uint32_t       RaytracingPipeline::getShaderHandleSizeAligned() const { return m_handleSizeAligned; }

} // namespace vzt
