#include "vzt/Pipeline.hpp"

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

    inline void Pipeline::setVertexInputDescription(VertexInputDescription vertexDescription)
    {
        m_vertexDescription = vertexDescription;
    }
    inline void Pipeline::addAttachment(ColorMask mask) { m_attachments.emplace_back(mask); }
    inline void Pipeline::addAttachments(std::size_t nb, ColorMask mask)
    {
        std::vector masks(nb, mask);
        m_attachments.insert(m_attachments.end(), masks.begin(), masks.end());
    }

    inline void            Pipeline::setTargetSize(Extent2D targetSize) { m_targetSize = std::move(targetSize); }
    inline const Extent2D& Pipeline::getTargetSize() const { return m_targetSize; }
    inline Extent2D&       Pipeline::getTargetSize() { return m_targetSize; }

    inline void            Pipeline::setViewport(Viewport config) { m_viewport = std::move(config); }
    inline const Viewport& Pipeline::getViewport() const { return m_viewport; }
    inline Viewport&       Pipeline::getViewport() { return m_viewport; }

    inline void Pipeline::setRasterization(Rasterization config) { m_rasterization = std::move(config); }
    inline const Rasterization& Pipeline::getRasterization() const { return m_rasterization; }
    inline Rasterization&       Pipeline::getRasterization() { return m_rasterization; }

    inline void                 Pipeline::setMultiSampling(MultiSampling config) { m_multiSample = std::move(config); }
    inline const MultiSampling& Pipeline::getMultiSampling() const { return m_multiSample; }
    inline MultiSampling&       Pipeline::getMultiSampling() { return m_multiSample; }

    inline void                Pipeline::setDepthStencil(DepthStencil config) { m_depthStencil = std::move(config); }
    inline const DepthStencil& Pipeline::getDepthStencil() const { return m_depthStencil; }
    inline DepthStencil&       Pipeline::getDepthStencil() { return m_depthStencil; }
} // namespace vzt
