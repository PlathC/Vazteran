#include "vzt/Vulkan/Pipeline/RaytracingPipeline.hpp"

namespace vzt
{
    inline void RaytracingPipeline::setDescriptorLayout(DescriptorLayout descriptorLayout)
    {
        m_descriptorLayout = std::move(descriptorLayout);
    }

    inline void RaytracingPipeline::setShaderGroup(const ShaderGroup& shaderGroup) { m_shaderGroup = shaderGroup; }

    inline VkPipeline       RaytracingPipeline::getHandle() const { return m_handle; }
    inline VkPipelineLayout RaytracingPipeline::getLayout() const { return m_pipelineLayout; }
    inline CSpan<uint8_t>   RaytracingPipeline::getShaderHandleStorage() const { return m_shaderHandleStorage; }
    inline uint32_t         RaytracingPipeline::getShaderHandleSize() const { return m_handleSize; }
    inline uint32_t         RaytracingPipeline::getShaderHandleSizeAligned() const { return m_handleSizeAligned; }

} // namespace vzt
