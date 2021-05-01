#ifndef VAZTERAN_FRAMEBUFFER_HPP
#define VAZTERAN_FRAMEBUFFER_HPP

namespace vzt {
    class LogicalDevice;
    class SwapChain;

    class FrameBuffer {
    public:
        FrameBuffer(LogicalDevice* logicalDevice, VkRenderPass rendPass, VkImageView imageView,
                    uint32_t width, uint32_t height);

        VkFramebuffer Buffer() const { return m_frameBuffer; }

        ~FrameBuffer();
    private:
        LogicalDevice* m_logicalDevice;
        VkFramebuffer m_frameBuffer;
    };
}

#endif //VAZTERAN_FRAMEBUFFER_HPP
