#include "vzt/Vulkan/Pipeline/RaytracingPipeline.hpp"

namespace vzt
{
    inline void RaytracingPipeline::setDescriptorLayout(DescriptorLayout descriptorLayout)
    {
        m_descriptorLayout = std::move(descriptorLayout);
    }
} // namespace vzt
