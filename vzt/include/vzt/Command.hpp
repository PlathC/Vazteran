#ifndef VZT_COMMAND_HPP
#define VZT_COMMAND_HPP

#include <vector>

#include <vulkan/vulkan_core.h>

#include "vzt/Core/Type.hpp"

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

        void                    flush();
        inline VkCommandBuffer& getHandle();

      private:
        CommandBuffer(VkCommandBuffer& handle);

        VkCommandBuffer* m_handle;
        bool             m_flushed = false;
    };
    class CommandPool
    {
      public:
        CommandPool() = default;
        CommandPool(View<Device> device, View<Queue> queue, uint32_t bufferNb);

        CommandPool(CommandPool&)                  = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        CommandPool(CommandPool&& other) noexcept;
        CommandPool& operator=(CommandPool&& other) noexcept;

        ~CommandPool();

        CommandBuffer operator[](const uint32_t bufferNumber);

        void allocateCommandBuffers(const uint32_t count);

      private:
        VkCommandPool m_handle{};

        View<Device> m_device;
        View<Queue>  m_queue;

        std::vector<VkCommandBuffer> m_commandBuffers;
    };
} // namespace vzt

#include "vzt/Command.inl"

#endif // VZT_COMMAND_HPP
