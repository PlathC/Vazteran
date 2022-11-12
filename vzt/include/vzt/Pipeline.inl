#include "vzt/Pipeline.hpp"

namespace vzt
{
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
