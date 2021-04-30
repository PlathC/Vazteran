#ifndef VAZTERAN_FRAMEBUFFER_HPP
#define VAZTERAN_FRAMEBUFFER_HPP

namespace vzt {
    class DeviceManager;
    class SwapChain;

    class FrameBuffer {
    public:
        FrameBuffer(DeviceManager* deviceManager, VkRenderPass rendPass, VkImageView imageView,
                    uint32_t width, uint32_t height);
        VkFramebuffer Buffer() const { return m_frameBuffer; }
        ~FrameBuffer();
    private:
        DeviceManager* m_deviceManager;
        VkFramebuffer m_frameBuffer;
    };
}

#endif //VAZTERAN_FRAMEBUFFER_HPP
