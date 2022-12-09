#include "vzt/Utils/RenderGraph.hpp"

namespace vzt
{
    template <class DerivedHandler, class... Args>
    void Pass::setRecordFunction(Args&&... args)
    {
        static_assert(std::is_base_of_v<RecordHandler, DerivedHandler>,
                      "DerivedHandler must inherit from RecordHandler.");
        m_recordCallback = std::make_unique<DerivedHandler>(std::forward<Args>(args)...);
    }

    inline View<Queue> Pass::getQueue() const { return m_queue; }
    inline void        Pass::setDescriptorLayout(DescriptorLayout&& layout) { m_descriptorLayout = std::move(layout); }
    inline const DescriptorLayout& Pass::getDescriptorLayout() const { return m_descriptorLayout; }
    inline DescriptorLayout&       Pass::getDescriptorLayout() { return m_descriptorLayout; }
    inline DescriptorPool&         Pass::getDescriptorPool() { return m_pool; }
    inline View<RenderPass>        Pass::getRenderPass() const { return &m_renderPass; }
} // namespace vzt
