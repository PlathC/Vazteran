#include "vzt/Command.hpp"

#include <cassert>

#include "vzt/Device.hpp"

namespace vzt
{
    CommandBuffer::CommandBuffer(VkCommandBuffer handle) : m_handle(handle)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkCheck(vkBeginCommandBuffer(handle, &beginInfo), "Failed to start the recording of the command buffer");
    }

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_flushed, other.m_flushed);
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_flushed, other.m_flushed);

        return *this;
    }

    CommandBuffer::~CommandBuffer()
    {
        if (m_handle)
            flush();
    }

    void CommandBuffer::barrier(PipelineBarrier barrier)
    {
        std::vector<VkImageMemoryBarrier> imageBarriers{};
        imageBarriers.reserve(barrier.imageBarriers.size());

        for (const auto& baseBarrier : barrier.imageBarriers)
        {
            VkImageMemoryBarrier imageBarrier{};
            imageBarrier.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.oldLayout = toVulkan(baseBarrier.oldLayout);
            imageBarrier.newLayout = toVulkan(baseBarrier.newLayout);
            imageBarrier.srcQueueFamilyIndex =
                baseBarrier.srcQueue ? baseBarrier.srcQueue->getId() : VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.dstQueueFamilyIndex =
                baseBarrier.dstQueue ? baseBarrier.dstQueue->getId() : VK_QUEUE_FAMILY_IGNORED;

            imageBarrier.image = baseBarrier.image;

            // TODO: Rewrite based on image properties
            imageBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBarrier.subresourceRange.baseMipLevel   = 0;
            imageBarrier.subresourceRange.levelCount     = 1;
            imageBarrier.subresourceRange.baseArrayLayer = 0;
            imageBarrier.subresourceRange.layerCount     = 1;

            imageBarriers.emplace_back(std::move(imageBarrier));
        }

        std::vector<VkBufferMemoryBarrier> bufferBarriers{};
        bufferBarriers.reserve(barrier.bufferBarriers.size());
        for (const auto& baseBarrier : barrier.bufferBarriers)
        {
            VkBufferMemoryBarrier bufferBarrier{};
            bufferBarrier.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.srcAccessMask = toVulkan(baseBarrier.src);
            bufferBarrier.dstAccessMask = toVulkan(baseBarrier.dst);
            bufferBarrier.srcQueueFamilyIndex =
                baseBarrier.srcQueue ? baseBarrier.srcQueue->getId() : VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex =
                baseBarrier.dstQueue ? baseBarrier.dstQueue->getId() : VK_QUEUE_FAMILY_IGNORED;

            bufferBarriers.emplace_back(std::move(bufferBarrier));
        }

        vkCmdPipelineBarrier(m_handle, toVulkan(barrier.src), toVulkan(barrier.dst), toVulkan(barrier.dependency), 0,
                             nullptr, static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
                             static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());
    }

    void CommandBuffer::barrier(PipelineStage src, PipelineStage dst, ImageBarrier imageBarrier)
    {
        PipelineBarrier pipelineBarrier{src, dst, std::vector{std::move(imageBarrier)}};
        barrier(std::move(pipelineBarrier));
    }

    void CommandBuffer::barrier(PipelineStage src, PipelineStage dst, BufferBarrier bufferBarrier)
    {
        PipelineBarrier pipelineBarrier{src, dst, {}, std::vector{std::move(bufferBarrier)}};
        barrier(std::move(pipelineBarrier));
    }

    void CommandBuffer::copy(const Buffer& src, const Buffer& dst, uint64_t size, uint64_t srcOffset,
                             uint64_t dstOffset)
    {
        VkBufferCopy copyRegion{};
        copyRegion.size      = size;
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        vkCmdCopyBuffer(m_handle, src.getHandle(), dst.getHandle(), 1, &copyRegion);
    }

    void CommandBuffer::flush()
    {
        if (m_flushed)
            return;

        vkCheck(vkEndCommandBuffer(m_handle), "Failed to end command buffer recording");

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
        if (m_handle == VK_NULL_HANDLE || m_commandBuffers.empty())
            return;

        // Avoid deleting command buffers while they're being processed by the device;
        m_device->wait();

        const uint32_t commandBufferNb = static_cast<uint32_t>(m_commandBuffers.size());
        vkFreeCommandBuffers(m_device->getHandle(), m_handle, commandBufferNb, m_commandBuffers.data());
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
