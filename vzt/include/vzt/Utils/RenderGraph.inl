#include "vzt/Utils/RenderGraph.hpp"

namespace vzt
{
    inline View<Queue> Pass::getQueue() const { return m_queue; }
    inline void        Pass::setDescriptorLayout(DescriptorLayout&& layout) { m_descriptorLayout = std::move(layout); }
    inline const DescriptorLayout& Pass::getDescriptorLayout() const { return m_descriptorLayout; }
    inline DescriptorLayout&       Pass::getDescriptorLayout() { return m_descriptorLayout; }
} // namespace vzt
