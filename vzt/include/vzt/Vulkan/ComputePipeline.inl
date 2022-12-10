#include "vzt/Vulkan/ComputePipeline.hpp"

namespace vzt
{
    inline void ComputePipeline::setProgram(const Program& program) { m_program = program; }
    inline void ComputePipeline::setDescriptorLayout(DescriptorLayout descriptorLayout)
    {
        m_descriptorLayout = descriptorLayout;
        m_descriptorLayout.compile();
    }

    inline VkPipeline       ComputePipeline::getHandle() const { return m_handle; }
    inline VkPipelineLayout ComputePipeline::getLayout() const { return m_pipelineLayout; }

} // namespace vzt
