#include "vzt/vulkan/descriptor.hpp"

namespace vzt
{
    inline DescriptorLayout::Bindings&       DescriptorLayout::getBindings() { return m_bindings; }
    inline const DescriptorLayout::Bindings& DescriptorLayout::getBindings() const { return m_bindings; }
    inline uint32_t                          DescriptorLayout::size() const { return uint32_t(m_bindings.size()); }
    inline VkDescriptorSetLayout             DescriptorLayout::getHandle() const { return m_handle; }
    inline VkDescriptorSet                   DescriptorSet::getHandle() const { return m_handle; }

    inline const std::vector<DescriptorType> DescriptorPool::DefaultDescriptors = {};

    DescriptorPoolBuilder& DescriptorPoolBuilder::add(DescriptorType type)
    {
        descriptorTypes.emplace(type);
        return *this;
    }

    DescriptorPoolBuilder& DescriptorPoolBuilder::setMaxSetNb(uint32_t mmaxSetNb)
    {
        maxSetNb = mmaxSetNb;
        return *this;
    }

    DescriptorPoolBuilder& DescriptorPoolBuilder::setMaxSetPerTypeNb(uint32_t mmaxSetPerTypeNb)
    {
        maxPerTypeNb = mmaxSetPerTypeNb;
        return *this;
    }

    DescriptorPoolBuilder& DescriptorPoolBuilder::addFlag(DescriptorPoolCreateFlag fflags)
    {
        flags |= fflags;
        return *this;
    }

    inline DescriptorSet DescriptorPool::operator[](uint32_t i) const { return m_descriptors[i]; }
    inline uint32_t      DescriptorPool::getRemaining() const
    {
        return static_cast<uint32_t>(m_maxSetNb - m_descriptors.size());
    }
    inline uint32_t DescriptorPool::getMaxSetNb() const { return m_maxSetNb; }
} // namespace vzt
