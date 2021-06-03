#ifndef VAZTERAN_COMMANDPOOL_HPP
#define VAZTERAN_COMMANDPOOL_HPP

#include <array>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

#include "Vazteran/Vulkan/FrameBuffer.hpp"

namespace vzt {
    class LogicalDevice;
    class GraphicPipeline;

    using RenderFunction = std::function<void(VkCommandBuffer, vzt::GraphicPipeline*, uint32_t)>;

    class CommandPool {
    public:
        CommandPool(LogicalDevice* logicalDevice, uint32_t bufferCount, vzt::RenderFunction renderFunction = {});

        CommandPool(CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        CommandPool(CommandPool&& other) noexcept;
        CommandPool& operator=(CommandPool&& other) noexcept;

        void AllocateCommandBuffers(uint32_t count);
        void SetRenderFunction(vzt::RenderFunction renderFunction);
        void RecordBuffer(const VkExtent2D& swapChainExtent, vzt::GraphicPipeline* graphicPipeline,
                const FrameBuffer& frameBuffer, uint32_t bufferNumber);

        VkCommandBuffer& operator[](uint32_t bufferNumber);

        ~CommandPool();
    private:
        LogicalDevice* m_logicalDevice;
        VkCommandPool m_vkHandle;
        std::vector<VkCommandBuffer> m_commandBuffers;
        uint32_t m_bufferCount;

        RenderFunction m_renderFunction;
    };
}

#endif //VAZTERAN_COMMANDPOOL_HPP
