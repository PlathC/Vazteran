#include "vzt/Vulkan/Pipeline/Pipeline.hpp"

namespace vzt
{
    Pipeline::Pipeline(View<Device> device) : DeviceObject<VkPipeline>(device) {}

    inline Pipeline::Pipeline(Pipeline&& other) noexcept : DeviceObject<VkPipeline>(std::move(other))
    {
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
    }

    inline Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
    {
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);

        DeviceObject<VkPipeline>::operator=(std::move(other));
        return *this;
    }

    const DescriptorLayout& Pipeline::getDescriptorLayout() const { return m_descriptorLayout; }
    inline VkPipelineLayout Pipeline::getLayout() const { return m_pipelineLayout; }
} // namespace vzt