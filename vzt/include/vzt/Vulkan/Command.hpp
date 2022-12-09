#ifndef VZT_VULKAN_COMMAND_HPP
#define VZT_VULKAN_COMMAND_HPP

#include <vector>

#include <vulkan/vulkan_core.h>

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/GraphicPipeline.hpp"

namespace vzt
{
    class CommandPool;
    class Device;
    class FrameBuffer;
    class Queue;

    class CommandBuffer
    {
      public:
        friend CommandPool;

        CommandBuffer(const CommandBuffer&)            = default;
        CommandBuffer& operator=(const CommandBuffer&) = default;

        CommandBuffer(CommandBuffer&& other) noexcept            = default;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept = default;

        ~CommandBuffer() = default;

        void barrier(PipelineBarrier barrier);
        void barrier(PipelineStage src, PipelineStage dst, ImageBarrier barrier);
        void barrier(PipelineStage src, PipelineStage dst, BufferBarrier barrier);

        void clear(View<Image> image, ImageLayout layout, Vec4 clearColor = {});
        void copy(const Buffer& src, const Buffer& dst, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0);

        void bind(const GraphicPipeline& graphicPipeline);
        void bind(const GraphicPipeline& graphicPipeline, const DescriptorSet& set);

        void bindVertexBuffer(const Buffer& buffer);
        void bindIndexBuffer(const Buffer& buffer, std::size_t index);

        void draw(uint32_t count, uint32_t offset = 0);
        void drawIndexed(const Buffer& indexBuffer, const Range<>& range);

        void setViewport(const Extent2D& size, float minDepth = 0.f, float maxDepth = 1.f);
        void setScissor(const Extent2D& size, Vec2i offset = {0u, 0u});

        void beginPass(const RenderPass& pass, const FrameBuffer& frameBuffer);
        void endPass();

        void                   begin();
        void                   end();
        inline VkCommandBuffer getHandle() const;

      private:
        CommandBuffer(VkCommandBuffer handle);

        VkCommandBuffer m_handle = nullptr;
    };
    class CommandPool
    {
      public:
        CommandPool() = default;
        CommandPool(View<Device> device, View<Queue> queue, uint32_t bufferNb = 1);

        CommandPool(CommandPool&)                  = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        CommandPool(CommandPool&& other) noexcept;
        CommandPool& operator=(CommandPool&& other) noexcept;

        ~CommandPool();

        CommandBuffer operator[](uint32_t bufferNumber);
        void          allocateCommandBuffers(uint32_t count);

      private:
        VkCommandPool m_handle{};

        View<Device> m_device;
        View<Queue>  m_queue;

        std::vector<VkCommandBuffer> m_commandBuffers;
    };
} // namespace vzt

#include "vzt/Vulkan/Command.inl"

#endif // VZT_VULKAN_COMMAND_HPP
