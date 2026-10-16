#include "render_graph.hpp"

namespace vzt
{
    template <class Type>
    StorageBuilder StorageBuilder::fromType(BufferUsage usage, MemoryLocation location, bool mappable)
    {
        return StorageBuilder(sizeof(Type), usage, location, mappable);
    }

    inline View<Buffer> PassContext::buffer(Handle storage) const { return graph.getStorage(frame, storage); }

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

    inline std::unique_ptr<Pass>&       TaskGraph::operator[](uint32_t passId) { return m_passes[passId]; }
    inline const std::unique_ptr<Pass>& TaskGraph::operator[](uint32_t passId) const { return m_passes[passId]; }
    inline uint32_t                     TaskGraph::size() const { return uint32_t(m_passes.size()); }
    inline std::vector<std::unique_ptr<Pass>>::iterator       TaskGraph::begin() { return m_passes.begin(); }
    inline std::vector<std::unique_ptr<Pass>>::iterator       TaskGraph::end() { return m_passes.end(); }
    inline std::vector<std::unique_ptr<Pass>>::const_iterator TaskGraph::begin() const { return m_passes.begin(); }
    inline std::vector<std::unique_ptr<Pass>>::const_iterator TaskGraph::end() const { return m_passes.end(); }

    inline Format       TaskGraph::getBackbufferFormat() const { return m_backbufferFormat; }
    inline Extent2D     TaskGraph::getBackbufferExtent() const { return m_backbufferExtent; }
    inline uint32_t     TaskGraph::getFrameNb() const { return m_frameNb; }
    inline uint32_t     TaskGraph::getBackbufferImageNb() const { return uint32_t(m_externalBackbuffers.size()); }
    inline View<Device> TaskGraph::getDevice() const { return m_device; }

    inline void  TaskGraph::setUserData(void* userData) { m_userData = userData; }
    inline void* TaskGraph::getUserData() const { return m_userData; }
} // namespace vzt
