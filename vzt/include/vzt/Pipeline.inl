#include "vzt/Pipeline.hpp"

namespace vzt
{
    inline void            Pipeline::setViewport(Viewport viewport) { m_viewport = viewport; }
    inline const Viewport& Pipeline::getViewport() const { return m_viewport; }

} // namespace vzt
