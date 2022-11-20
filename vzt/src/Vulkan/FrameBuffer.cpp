#include "vzt/Vulkan/FrameBuffer.hpp"

#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/RenderPass.hpp"

namespace vzt
{
    FrameBuffer::FrameBuffer(View<Device> device, Extent2D size) : m_device(device), m_size(size) {}

    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_size, other.m_size);
        std::swap(m_attachments, other.m_attachments);
    }

    FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_size, other.m_size);
        std::swap(m_attachments, other.m_attachments);

        return *this;
    }

    FrameBuffer::~FrameBuffer()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        vkDestroyFramebuffer(m_device->getHandle(), m_handle, nullptr);
    }

    void FrameBuffer::addAttachment(ImageView attachment) { m_attachments.emplace_back(std::move(attachment)); }

    void FrameBuffer::compile(View<RenderPass> renderPass)
    {
        assert(m_handle == VK_NULL_HANDLE && "Frame buffer is already compiled");

        std::vector<VkImageView> attachmentView{};
        attachmentView.reserve(m_attachments.size());
        for (const auto& view : m_attachments)
            attachmentView.emplace_back(view.getHandle());

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass      = renderPass->getHandle();
        frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachmentView.size());
        frameBufferInfo.pAttachments    = attachmentView.data();
        frameBufferInfo.width           = m_size.width;
        frameBufferInfo.height          = m_size.height;
        frameBufferInfo.layers          = 1;

        vkCheck(vkCreateFramebuffer(m_device->getHandle(), &frameBufferInfo, nullptr, &m_handle),
                "Failed to create frame buffer!");
    }
} // namespace vzt
