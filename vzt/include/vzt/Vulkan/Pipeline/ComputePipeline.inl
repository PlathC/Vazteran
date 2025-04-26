#include "vzt/Vulkan/Pipeline/ComputePipeline.hpp"

namespace vzt
{
    inline VkPipeline       ComputePipeline::getHandle() const { return m_handle; }
    inline VkPipelineLayout ComputePipeline::getLayout() const { return m_pipelineLayout; }

} // namespace vzt
