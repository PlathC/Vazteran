#ifndef VAZTERAN_FRAMEBUFFER_HPP
#define VAZTERAN_FRAMEBUFFER_HPP

namespace vzt {
    class LogicalDevice;
    class SwapChain;

    class FrameBuffer {
    public:
        FrameBuffer(vzt::LogicalDevice* logicalDevice, VkRenderPass renderPass, std::vector<VkImageView> attachments,
                    uint32_t width, uint32_t height);

        FrameBuffer(FrameBuffer&) = delete;
        FrameBuffer& operator=(FrameBuffer&) = delete;

        FrameBuffer(FrameBuffer&&) noexcept;
        FrameBuffer& operator=(FrameBuffer&&) noexcept;

        VkFramebuffer VkHandle() const { return m_vkHandle; }

        ~FrameBuffer();
    private:
        vzt::LogicalDevice* m_logicalDevice;
        VkFramebuffer m_vkHandle;
    };
}

#endif //VAZTERAN_FRAMEBUFFER_HPP
