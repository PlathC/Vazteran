#ifndef VZT_VULKAN_COMMAND_HPP
#define VZT_VULKAN_COMMAND_HPP

#include <vector>

#include <vulkan/vulkan_core.h>

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/Pipeline.hpp"

namespace vzt
{
    class CommandPool;
    class Device;
    class Queue;

    class CommandBuffer
    {
      public:
        friend CommandPool;

        CommandBuffer(const CommandBuffer&)            = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&& other) noexcept;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept;

        ~CommandBuffer();

        void barrier(PipelineBarrier barrier);
        void barrier(PipelineStage src, PipelineStage dst, ImageBarrier barrier);
        void barrier(PipelineStage src, PipelineStage dst, BufferBarrier barrier);

        void copy(const Buffer& src, const Buffer& dst, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0);

        void                   flush();
        inline VkCommandBuffer getHandle() const;

      private:
        CommandBuffer(VkCommandBuffer handle);

        VkCommandBuffer m_handle  = nullptr;
        bool            m_flushed = false;
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
