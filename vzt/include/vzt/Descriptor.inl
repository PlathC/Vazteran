#include "vzt/Descriptor.hpp"

namespace vzt
{
    inline VkDescriptorSetLayout DescriptorLayout::getHandle() const { return m_handle; }

    inline const std::vector<DescriptorType> DescriptorPool::DefaultDescriptors = {DescriptorType::UniformBuffer};

    inline DescriptorSet DescriptorPool::operator[](uint32_t i) const { return m_descriptors[i]; }
    inline uint32_t                      DescriptorPool::getRemaining() const
    {
        return static_cast<uint32_t>(m_maxSetNb - m_descriptors.size());
    }
    inline uint32_t         DescriptorPool::getMaxSetNb() const { return m_maxSetNb; }
    inline VkDescriptorPool DescriptorPool::getHandle() const { return m_handle; }
} // namespace vzt
