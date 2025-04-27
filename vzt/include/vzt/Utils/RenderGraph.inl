#include "vzt/Utils/RenderGraph.hpp"

namespace vzt
{
    template <class Type>
    StorageBuilder StorageBuilder::fromType(BufferUsage usage, MemoryLocation location, bool mappable)
    {
        return StorageBuilder(sizeof(Type), usage, location, mappable);
    }

    template <class DerivedHandler, class... Args>
    void Pass::setRecordFunction(Args&&... args)
    {
        static_assert(std::is_base_of_v<RecordHandler, DerivedHandler>,
                      "DerivedHandler must inherit from RecordHandler.");
        m_recordCallback = std::make_unique<DerivedHandler>(std::forward<Args>(args)...);
    }

    inline std::string_view        Pass::getName() const { return m_name; }
    inline const DescriptorLayout& Pass::getDescriptorLayout() const { return m_descriptorLayout; }
    inline DescriptorPool&         Pass::getDescriptorPool() { return m_pool; }
    inline View<RenderPass>        Pass::getRenderPass() const { return &m_renderPass; }

    inline ComputePipeline& ComputePass::getPipeline() { return m_pipeline; }
    inline GraphicPipeline& GraphicsPass::getPipeline() { return m_pipeline; }

    inline std::unique_ptr<Pass>&       RenderGraph::operator[](uint32_t passId) { return m_passes[passId]; }
    inline const std::unique_ptr<Pass>& RenderGraph::operator[](uint32_t passId) const { return m_passes[passId]; }
    inline uint32_t                     RenderGraph::size() const { return uint32_t(m_passes.size()); }
    inline std::vector<std::unique_ptr<Pass>>::iterator       RenderGraph::begin() { return m_passes.begin(); }
    inline std::vector<std::unique_ptr<Pass>>::iterator       RenderGraph::end() { return m_passes.end(); }
    inline std::vector<std::unique_ptr<Pass>>::const_iterator RenderGraph::begin() const { return m_passes.begin(); }
    inline std::vector<std::unique_ptr<Pass>>::const_iterator RenderGraph::end() const { return m_passes.end(); }
    inline View<Device>                                       RenderGraph::getDevice() const { return m_device; }
} // namespace vzt
