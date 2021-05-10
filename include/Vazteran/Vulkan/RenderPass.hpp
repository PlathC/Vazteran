
#ifndef VAZTERAN_RENDERPASS_HPP
#define VAZTERAN_RENDERPASS_HPP

#include <vulkan/vulkan.h>

namespace vzt {
    class LogicalDevice;

    class RenderPass {
    public:
        RenderPass(vzt::LogicalDevice* logicalDevice, VkFormat colorImageFormat);

        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        RenderPass(RenderPass&& other) noexcept;
        RenderPass& operator=(RenderPass&& other) noexcept;

        VkRenderPass VkHandle() const { return m_vkHandle; }

        ~RenderPass();
    private:
        VkRenderPass m_vkHandle;
        vzt::LogicalDevice* m_logicalDevice;
    };
}


#endif //VAZTERAN_RENDERPASS_HPP
