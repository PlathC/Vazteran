#ifndef VZT_VULKAN_COMMAND_HPP
#define VZT_VULKAN_COMMAND_HPP

#include <vector>

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/Pipeline/GraphicsPipeline.hpp"
#include "vzt/Vulkan/Pipeline/RaytracingPipeline.hpp"

namespace vzt
{
    struct AccelerationStructureBuilder;
    class CommandPool;
    class ComputePipeline;
    class Device;
    class FrameBuffer;
    class QueryPool;
    class Queue;

    struct Blit
    {
        Extent3D    srcOffsets[2];
        ImageAspect srcAspect;
        uint32_t    srcMipLevel;

        Extent3D    dstOffsets[2];
        ImageAspect dstAspect;
        uint32_t    dstMipLevel;
    };

    class CommandBuffer : public DeviceObject<VkCommandBuffer>
    {
      public:
        friend CommandPool;

        CommandBuffer(const CommandBuffer&)            = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&& other) noexcept            = default;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept = default;

        ~CommandBuffer() override = default;

        void begin();
        void end();

        void barrier(const PipelineBarrier& barrier);
        void barrier(PipelineStage src, PipelineStage dst, ImageBarrier barrier);
        void barrier(PipelineStage src, PipelineStage dst, BufferBarrier barrier);

        void clear(View<DeviceImage> image, ImageLayout layout, Vec4 clearColor = {});
        void blit(View<DeviceImage> src, ImageLayout srcLayout, ImageAspect srcAspect, Vec2u srcStart, Vec2u srcEnd,
                  View<DeviceImage> dst, ImageLayout dstLayout, ImageAspect dstAspect, Vec2u dstStart, Vec2u dstEnd,
                  Filter filter = Filter::Linear);
        void blit(View<DeviceImage> src, ImageLayout srcLayout, ImageAspect srcAspect, View<DeviceImage> dst,
                  ImageLayout dstLayout, ImageAspect dstAspect, Filter filter = Filter::Linear);
        void blit(View<DeviceImage> src, ImageLayout srcLayout, View<DeviceImage> dst, ImageLayout dstLayout,
                  Filter filter = Filter::Linear);
        void blit(View<DeviceImage> src, ImageLayout srcLayout, View<DeviceImage> dst, ImageLayout dstLayout,
                  Filter filter, const Blit& blit);
        void copy(View<Buffer> src, View<Buffer> dst, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0);
        void copy(View<Buffer> src, View<DeviceImage> dst, uint32_t width, uint32_t height,
                  ImageAspect aspect = ImageAspect::Color);
        void copy(View<DeviceImage> src, View<DeviceImage> dst, uint32_t width, uint32_t height,
                  ImageAspect aspect = ImageAspect::Color);

        void bind(const GraphicPipeline& graphicPipeline);
        void bind(const GraphicPipeline& graphicPipeline, const DescriptorSet& set);
        void bind(const ComputePipeline& computePipeline);
        void bind(const ComputePipeline& computePipeline, const DescriptorSet& set);
        void bind(const RaytracingPipeline& raytracingPipeline);
        void bind(const RaytracingPipeline& raytracingPipeline, const DescriptorSet& set);
        void bindVertexBuffer(const Buffer& buffer);
        void bindIndexBuffer(const Buffer& buffer, std::size_t index);

        void dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1);
        void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t vertexOffset = 0,
                  uint32_t instanceOffset = 0);
        void drawIndexed(const Buffer& indexBuffer, const Range<>& range, uint32_t instanceCount = 1,
                         int32_t vertexOffset = 0, uint32_t instanceOffset = 0);

        void drawIndirect(const BufferCSpan& buffer, uint32_t drawCount, uint32_t stride);
        void drawIndexedIndirect(const BufferCSpan& buffer, uint32_t drawCount, uint32_t stride);
        void dispatchIndirect(const BufferCSpan& buffer);

        void traceRays(StridedSpan<uint64_t> raygen, StridedSpan<uint64_t> miss, StridedSpan<uint64_t> hit,
                       StridedSpan<uint64_t> callable, uint32_t width, uint32_t height, uint32_t depth = 1);

        void setViewport(const Extent2D& size, float minDepth = 0.f, float maxDepth = 1.f);
        void setScissor(const Extent2D& size, Vec2i offset = {0u, 0u});

        void reset(const QueryPool& pool, uint32_t firstQuery, uint32_t queryCount);
        void writeTimeStamp(const QueryPool& pool, uint32_t query, PipelineStage waitingStage);

        void beginPass(const RenderPass& pass, const FrameBuffer& frameBuffer);
        void endPass();

        void buildAs(AccelerationStructureBuilder& builder);

      private:
        CommandBuffer(View<Device> m_device, VkCommandBuffer handle);
    };

    class CommandPool : public DeviceObject<VkCommandPool>
    {
      public:
        CommandPool() = default;
        CommandPool(View<Device> device, View<Queue> queue, uint32_t bufferNb = 1);

        CommandPool(CommandPool&)                  = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        CommandPool(CommandPool&& other) noexcept;
        CommandPool& operator=(CommandPool&& other) noexcept;

        ~CommandPool() override;

        CommandBuffer operator[](uint32_t bufferNumber);
        void          allocateCommandBuffers(uint32_t count);

      private:
        View<Queue>                  m_queue;
        std::vector<VkCommandBuffer> m_commandBuffers;
    };
} // namespace vzt

#endif // VZT_VULKAN_COMMAND_HPP
