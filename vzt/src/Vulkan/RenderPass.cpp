#include "vzt/Vulkan/RenderPass.hpp"

#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    VkAttachmentDescription toVulkan(const AttachmentUse& configuration)
    {
        VkAttachmentDescription description{};
        description.initialLayout  = toVulkan(configuration.initialLayout);
        description.finalLayout    = toVulkan(configuration.finalLayout);
        description.stencilLoadOp  = toVulkan(configuration.stencilLoapOp);
        description.stencilStoreOp = toVulkan(configuration.stencilStoreOp);
        description.loadOp         = toVulkan(configuration.loadOp);
        description.storeOp        = toVulkan(configuration.storeOp);
        description.format         = toVulkan(configuration.format);
        description.samples        = static_cast<VkSampleCountFlagBits>(configuration.sampleCount);

        return description;
    }

    RenderPass::RenderPass(View<Device> device) : DeviceObject<VkRenderPass>(device) {}

    RenderPass::RenderPass(RenderPass&& other) noexcept : DeviceObject<VkRenderPass>(std::move(other))
    {
        std::swap(m_inputAttachments, other.m_inputAttachments);
        std::swap(m_colorAttachments, other.m_colorAttachments);
        std::swap(m_depthAttachment, other.m_depthAttachment);
        std::swap(m_dependencies, other.m_dependencies);
    }
    RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
    {
        std::swap(m_inputAttachments, other.m_inputAttachments);
        std::swap(m_colorAttachments, other.m_colorAttachments);
        std::swap(m_depthAttachment, other.m_depthAttachment);
        std::swap(m_dependencies, other.m_dependencies);

        DeviceObject<VkRenderPass>::operator=(std::move(other));
        return *this;
    }

    RenderPass::~RenderPass()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroyRenderPass(m_device->getHandle(), m_handle, nullptr);
    }

    void RenderPass::addInput(AttachmentUse input) { m_inputAttachments.emplace_back(input); }
    void RenderPass::addColor(AttachmentUse color) { m_colorAttachments.emplace_back(color); }
    void RenderPass::setDepth(AttachmentUse depth) { m_depthAttachment = depth; }
    void RenderPass::addDependency(SubpassDependency dependency) { m_dependencies.emplace_back(dependency); }
    void RenderPass::compile()
    {
        std::vector<VkClearValue> clearValues{};
        clearValues.resize(m_colorAttachments.size() + 1);
        std::vector<VkAttachmentDescription> attachments{};
        attachments.reserve(m_colorAttachments.size() + 1 + m_inputAttachments.size());

        std::vector<VkAttachmentReference> colorRefs{};
        colorRefs.resize(m_colorAttachments.size());
        for (std::size_t i = 0; i < m_colorAttachments.size(); i++)
        {
            const auto& attachment = m_colorAttachments[i];
            attachments.emplace_back(toVulkan(attachment));
            clearValues[i].color = {
                attachment.clearValue.x,
                attachment.clearValue.y,
                attachment.clearValue.z,
                attachment.clearValue.w,
            };

            VkAttachmentReference& currentAttachmentRef = colorRefs[i];
            currentAttachmentRef.attachment             = static_cast<uint32_t>(i);
            currentAttachmentRef.layout                 = toVulkan(attachment.usedLayout);
        }

        clearValues[attachments.size()].depthStencil = {m_depthAttachment.clearValue.x,
                                                        static_cast<uint32_t>(m_depthAttachment.clearValue.w)};
        attachments.emplace_back(toVulkan(m_depthAttachment));

        std::vector<VkAttachmentReference> inputRefs{};
        inputRefs.resize(m_inputAttachments.size());
        for (std::size_t i = 0; i < m_inputAttachments.size(); i++)
        {
            const auto& attachment = m_inputAttachments[i];
            attachments.emplace_back(toVulkan(attachment));

            VkAttachmentReference& currentAttachmentRef = inputRefs[i];
            currentAttachmentRef.attachment             = static_cast<uint32_t>(attachments.size() - 1);
            currentAttachmentRef.layout                 = toVulkan(attachment.usedLayout);
        }

        VkAttachmentReference depthRef;
        depthRef.attachment = static_cast<uint32_t>(m_colorAttachments.size());
        depthRef.layout     = toVulkan(m_depthAttachment.usedLayout);

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = static_cast<uint32_t>(colorRefs.size());
        subpass.pColorAttachments       = colorRefs.data();
        subpass.pDepthStencilAttachment = &depthRef;
        subpass.inputAttachmentCount    = static_cast<uint32_t>(inputRefs.size());
        subpass.pInputAttachments       = inputRefs.data();

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;

        std::vector<VkSubpassDependency> subpassDependencies{};
        subpassDependencies.resize(m_dependencies.size());
        for (std::size_t i = 0; i < m_dependencies.size(); i++)
        {
            subpassDependencies[i].srcSubpass      = m_dependencies[i].src;
            subpassDependencies[i].dstSubpass      = m_dependencies[i].dst;
            subpassDependencies[i].srcStageMask    = toVulkan(m_dependencies[i].srcStage);
            subpassDependencies[i].srcAccessMask   = toVulkan(m_dependencies[i].srcAccess);
            subpassDependencies[i].dstStageMask    = toVulkan(m_dependencies[i].dstStage);
            subpassDependencies[i].dstAccessMask   = toVulkan(m_dependencies[i].dstAccess);
            subpassDependencies[i].dependencyFlags = toVulkan(m_dependencies[i].dependencyFlags);
        }
        renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassInfo.pDependencies   = subpassDependencies.data();

        vkCheck(vkCreateRenderPass(m_device->getHandle(), &renderPassInfo, nullptr, &m_handle),
                "Failed to create render pass!");
    }
} // namespace vzt
