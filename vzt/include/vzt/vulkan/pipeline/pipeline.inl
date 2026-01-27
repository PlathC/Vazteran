#include "vzt/vulkan/pipeline/pipeline.hpp"

namespace vzt
{
    template <class Type>
    PushConstant PushConstant::Typed(ShaderStage stages)
    {
        return {stages, 0, sizeof(Type)};
    }

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

    inline void             Pipeline::add(PushConstant constant) { m_pushConstants.emplace_back(std::move(constant)); }
    const DescriptorLayout& Pipeline::getDescriptorLayout() const { return m_descriptorLayout; }
    inline VkPipelineLayout Pipeline::getLayout() const { return m_pipelineLayout; }
} // namespace vzt