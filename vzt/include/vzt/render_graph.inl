#include "render_graph.hpp"

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

    inline std::string_view Pass::getName() const { return m_name; }
    inline DescriptorPool&  Pass::getDescriptorPool() { return m_pool; }

    inline CSpan<View<ImageView>> Pass::getColorOutputs(uint32_t frame) const
    {
        assert(frame < m_graph->getFrameNb());

        const uint32_t outputNb = static_cast<uint32_t>(m_colorOutputs.size() / m_graph->getFrameNb());
        assert(outputNb > 0);

        return {m_colorOutputs.data() + outputNb * frame, outputNb};
    }

    inline View<ImageView> Pass::getDepth(uint32_t frame) const
    {
        assert(frame < m_graph->getFrameNb());
        assert(!m_depthOutputImageViews.empty());

        return m_depthOutputImageViews[frame];
    }

    inline ComputePipeline&         ComputePass::getPipeline() { return m_pipeline; }
    inline GraphicsPipeline&        GraphicsPass::getPipeline() { return m_pipeline; }
    inline GraphicsPipelineBuilder& GraphicsPass::getBuilder() { return m_graphicsPipelineBuilder; }

    inline std::unique_ptr<Pass>&       RenderGraph::operator[](uint32_t passId) { return m_passes[passId]; }
    inline const std::unique_ptr<Pass>& RenderGraph::operator[](uint32_t passId) const { return m_passes[passId]; }
    inline uint32_t                     RenderGraph::size() const { return uint32_t(m_passes.size()); }
    inline std::vector<std::unique_ptr<Pass>>::iterator       RenderGraph::begin() { return m_passes.begin(); }
    inline std::vector<std::unique_ptr<Pass>>::iterator       RenderGraph::end() { return m_passes.end(); }
    inline std::vector<std::unique_ptr<Pass>>::const_iterator RenderGraph::begin() const { return m_passes.begin(); }
    inline std::vector<std::unique_ptr<Pass>>::const_iterator RenderGraph::end() const { return m_passes.end(); }

    inline Format       RenderGraph::getBackbufferFormat() const { return m_backbufferFormat; }
    inline Extent2D     RenderGraph::getBackbufferExtent() const { return m_backbufferExtent; }
    inline uint32_t     RenderGraph::getFrameNb() const { return m_frameNb; }
    inline uint32_t     RenderGraph::getBackbufferImageNb() const { return uint32_t(m_externalBackbuffers.size()); }
    inline View<Device> RenderGraph::getDevice() const { return m_device; }

    inline void  RenderGraph::setUserData(void* userData) { m_userData = userData; }
    inline void* RenderGraph::getUserData() const { return m_userData; }
} // namespace vzt
