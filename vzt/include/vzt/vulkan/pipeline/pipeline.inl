#include "vzt/vulkan/pipeline/pipeline.hpp"

namespace vzt
{
    pipeline::pipeline(View<Device> device) : DeviceObject<VkPipeline>(device) {}

    inline pipeline::pipeline(pipeline&& other) noexcept : DeviceObject<VkPipeline>(std::move(other))
    {
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
    }

    inline pipeline& pipeline::operator=(pipeline&& other) noexcept
    {
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);

        DeviceObject<VkPipeline>::operator=(std::move(other));
        return *this;
    }

    const DescriptorLayout& pipeline::getDescriptorLayout() const { return m_descriptorLayout; }
    inline VkPipelineLayout pipeline::getLayout() const { return m_pipelineLayout; }
} // namespace vzt