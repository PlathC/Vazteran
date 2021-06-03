#ifndef VAZTERAN_FRAMEBUFFER_HPP
#define VAZTERAN_FRAMEBUFFER_HPP

namespace vzt {
    class LogicalDevice;
    class SwapChain;

    class FrameBuffer {
    public:
        FrameBuffer(vzt::LogicalDevice* logicalDevice, VkImage colorImage, VkImageView colorImageView,
                VkRenderPass renderPass, std::vector<VkImageView> attachments, uint32_t width, uint32_t height);

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        FrameBuffer(FrameBuffer&&) noexcept;
        FrameBuffer& operator=(FrameBuffer&&) noexcept;

        VkFramebuffer VkHandle() const { return m_vkHandle; }

        ~FrameBuffer();
    private:
        vzt::LogicalDevice* m_logicalDevice;
        VkFramebuffer m_vkHandle = VK_NULL_HANDLE;

        VkImage m_image;
        VkImageView m_imageView = VK_NULL_HANDLE;
    };
}

#endif //VAZTERAN_FRAMEBUFFER_HPP
