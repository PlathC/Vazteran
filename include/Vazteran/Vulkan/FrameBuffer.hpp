#ifndef VAZTERAN_FRAMEBUFFER_HPP
#define VAZTERAN_FRAMEBUFFER_HPP

namespace vzt {
    class LogicalDevice;
    class SwapChain;

    class FrameBuffer {
    public:
        FrameBuffer(LogicalDevice* logicalDevice, VkRenderPass renderPass, VkImageView imageView,
                    VkImageView depthImageView, uint32_t width, uint32_t height);

        VkFramebuffer VkHandle() const { return m_vkHandle; }

        ~FrameBuffer();
    private:
        LogicalDevice* m_logicalDevice;
        VkFramebuffer m_vkHandle;
    };
}

#endif //VAZTERAN_FRAMEBUFFER_HPP
