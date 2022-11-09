#include "vzt/Command.hpp"

#include <cassert>

#include "vzt/Device.hpp"

namespace vzt
{
    CommandBuffer::CommandBuffer(VkCommandBuffer& handle) : m_handle(&handle)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkCheck(vkBeginCommandBuffer(handle, &beginInfo), "Failed to start the recording of the command buffer");
    }

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept { std::swap(m_handle, other.m_handle); }
    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);

        return *this;
    }

    CommandBuffer::~CommandBuffer() { flush(); }

    void CommandBuffer::flush()
    {
        if (m_flushed)
            return;

        vkCheck(vkEndCommandBuffer(*m_handle), "Failed to end command buffer recording");

        m_flushed = true;
    }

    CommandPool::CommandPool(View<Device> device, View<Queue> queue, uint32_t bufferNb) : m_device(device)
    {
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = queue->getId();
        commandPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkCheck(vkCreateCommandPool(m_device->getHandle(), &commandPoolInfo, nullptr, &m_handle),
                "Failed to create command pool.");

        allocateCommandBuffers(bufferNb);
    }

    CommandPool::CommandPool(CommandPool&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_commandBuffers, other.m_commandBuffers);
    }

    CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_commandBuffers, other.m_commandBuffers);

        return *this;
    }

    CommandPool::~CommandPool()
    {
        if (!m_commandBuffers.empty())
        {
            const uint32_t commandBufferNb = static_cast<uint32_t>(m_commandBuffers.size());
            vkFreeCommandBuffers(m_device->getHandle(), m_handle, commandBufferNb, m_commandBuffers.data());
        }

        if (m_handle != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_device->getHandle(), m_handle, nullptr);
    }

    void CommandPool::allocateCommandBuffers(const uint32_t count)
    {
        if (!m_commandBuffers.empty())
        {
            const uint32_t commandBufferNb = static_cast<uint32_t>(m_commandBuffers.size());
            vkFreeCommandBuffers(m_device->getHandle(), m_handle, commandBufferNb, m_commandBuffers.data());
            m_commandBuffers.clear();
        }

        m_commandBuffers.resize(count);

        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool        = m_handle;
        commandBufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount = count;

        vkCheck(vkAllocateCommandBuffers(m_device->getHandle(), &commandBufferAllocInfo, m_commandBuffers.data()),
                "Failed to allocate command buffers");
    }

    CommandBuffer CommandPool::operator[](uint32_t bufferNumber)
    {
        assert(bufferNumber < m_commandBuffers.size() && "bufferNumber should be < than m_commandBuffers.size()");
        return CommandBuffer(m_commandBuffers[bufferNumber]);
    }
} // namespace vzt
