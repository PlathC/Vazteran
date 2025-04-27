#include "vzt/Vulkan/RenderPass.hpp"

namespace vzt
{
    inline const std::vector<AttachmentUse>& RenderPass::getInputAttachments() const { return m_inputAttachments; }
    inline const std::vector<AttachmentUse>& RenderPass::getColorAttachments() const { return m_colorAttachments; }
    inline const AttachmentUse&              RenderPass::getDepthAttachment() const { return m_depthAttachment; }
} // namespace vzt
