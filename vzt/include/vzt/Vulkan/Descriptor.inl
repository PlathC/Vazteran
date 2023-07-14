#include "vzt/Vulkan/Descriptor.hpp"

namespace vzt
{
    inline const DescriptorLayout::Bindings& DescriptorLayout::getBindings() const { return m_bindings; }
    inline uint32_t                          DescriptorLayout::size() const { return uint32_t(m_bindings.size()); }
    inline VkDescriptorSetLayout             DescriptorLayout::getHandle() const { return m_handle; }
    inline VkDescriptorSet                   DescriptorSet::getHandle() const { return m_handle; }

    inline const std::vector<DescriptorType> DescriptorPool::DefaultDescriptors = {DescriptorType::UniformBuffer};

    inline DescriptorSet DescriptorPool::operator[](uint32_t i) const { return m_descriptors[i]; }
    inline uint32_t      DescriptorPool::getRemaining() const
    {
        return static_cast<uint32_t>(m_maxSetNb - m_descriptors.size());
    }
    inline uint32_t         DescriptorPool::getMaxSetNb() const { return m_maxSetNb; }
    inline VkDescriptorPool DescriptorPool::getHandle() const { return m_handle; }
} // namespace vzt
