#include "vzt/Vulkan/Pipeline/GraphicsPipeline.hpp"

namespace vzt
{
    template <class Type>
    VertexBinding VertexBinding::Typed(uint32_t binding, VertexInputRate inputRate)
    {
        return {binding, sizeof(Type), inputRate};
    }

    inline VertexAttribute VertexBinding::getAttribute(uint32_t location, Format dataFormat, uint32_t offset)
    {
        return {offset, location, dataFormat, binding};
    }

    inline void VertexInputDescription::add(VertexBinding binding) { bindings.emplace_back(std::move(binding)); }
    inline void VertexInputDescription::add(VertexAttribute binding) { attributes.emplace_back(std::move(binding)); }
    inline void VertexInputDescription::add(uint32_t offset, uint32_t location, Format dataFormat, uint32_t binding)
    {
        add(VertexAttribute{offset, location, dataFormat, binding});
    }

    inline void GraphicPipeline::setVertexInputDescription(VertexInputDescription vertexDescription)
    {
        m_vertexDescription = vertexDescription;
    }
    inline void GraphicPipeline::addAttachmentColorBlend(ColorMask mask) { m_attachments.emplace_back(mask); }
    inline void GraphicPipeline::addAttachmentsColorBlend(std::size_t nb, ColorMask mask)
    {
        std::vector masks(nb, mask);
        m_attachments.insert(m_attachments.end(), masks.begin(), masks.end());
    }

    inline void            GraphicPipeline::setViewport(Viewport config) { m_viewport = std::move(config); }
    inline const Viewport& GraphicPipeline::getViewport() const { return m_viewport; }
    inline Viewport&       GraphicPipeline::getViewport() { return m_viewport; }

    inline void GraphicPipeline::setRasterization(Rasterization config) { m_rasterization = std::move(config); }
    inline const Rasterization& GraphicPipeline::getRasterization() const { return m_rasterization; }
    inline Rasterization&       GraphicPipeline::getRasterization() { return m_rasterization; }

    inline void GraphicPipeline::setMultiSampling(MultiSampling config) { m_multiSample = std::move(config); }
    inline const MultiSampling& GraphicPipeline::getMultiSampling() const { return m_multiSample; }
    inline MultiSampling&       GraphicPipeline::getMultiSampling() { return m_multiSample; }

    inline void GraphicPipeline::setDepthStencil(DepthStencil config) { m_depthStencil = std::move(config); }
    inline const DepthStencil& GraphicPipeline::getDepthStencil() const { return m_depthStencil; }
    inline DepthStencil&       GraphicPipeline::getDepthStencil() { return m_depthStencil; }

    inline void GraphicPipeline::setPrimitiveTopology(PrimitiveTopology topology) { m_primitiveTopology = topology; }
    inline const PrimitiveTopology& GraphicPipeline::getPrimitiveTopology() const { return m_primitiveTopology; }
    inline PrimitiveTopology&       GraphicPipeline::getPrimitiveTopology() { return m_primitiveTopology; }
} // namespace vzt
