#include "vzt/Vulkan/Pipeline/Pipeline.hpp"

namespace vzt
{
    inline Pipeline::Pipeline(Pipeline&& other) noexcept { std::swap(m_descriptorLayout, other.m_descriptorLayout); }
    inline Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
    {
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        return *this;
    }

    const DescriptorLayout& Pipeline::getDescriptorLayout() const { return m_descriptorLayout; }
} // namespace vzt