#include "vzt/Buffer.hpp"

#include <cassert>

#include "vzt/Command.hpp"
#include "vzt/Device.hpp"

namespace vzt
{
    VmaAllocationCreateFlags toVma(bool mappable)
    {
        VmaAllocationCreateFlags flags = 0;

        if (mappable)
            flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        return flags;
    }

    VkMemoryPropertyFlags toVma(MemoryLocation location)
    {
        if (location == MemoryLocation::Host)
            return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

        return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    Buffer::Buffer(View<Device> device, std::size_t byteNb, BufferUsage usage, MemoryLocation location, bool mappable)
        : m_device(device), m_size(byteNb), m_usage(usage), m_mappable(mappable)
    {
        VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size               = m_size;
        bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage              = toVulkan(usage);
        if (!mappable)
            bufferInfo.usage |= toVulkan(BufferUsage::TransferDst);

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags                   = toVma(mappable);
        allocInfo.preferredFlags          = toVma(location);

        vkCheck(vmaCreateBuffer(m_device->getAllocator(), &bufferInfo, &allocInfo, &m_handle, &m_allocation, nullptr),
                "Failed to create vertex buffer!");
    }

    Buffer::Buffer(Buffer&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_mappable, other.m_mappable);
        std::swap(m_size, other.m_size);
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_mappable, other.m_mappable);
        std::swap(m_size, other.m_size);

        return *this;
    }

    Buffer::~Buffer()
    {
        if (m_handle != VK_NULL_HANDLE)
            vmaDestroyBuffer(m_device->getAllocator(), m_handle, m_allocation);
    }

    uint8_t* Buffer::map()
    {
        uint8_t* data = nullptr;
        vmaMapMemory(m_device->getAllocator(), m_allocation, reinterpret_cast<void**>(&data));

        return data;
    }

    void Buffer::unMap() { vmaUnmapMemory(m_device->getAllocator(), m_allocation); }

    void Buffer::update(CSpan<uint8_t> newData, const std::size_t offset)
    {
        assert(newData.size < m_size && "Update must be less than the size of the buffer.");

        if (m_mappable)
        {
            uint8_t* data = map();
            std::memcpy(data + offset, newData.data, newData.size);
            unMap();
        }
        else
        {
            auto transferBuffer = fromData(m_device, newData, BufferUsage::TransferSrc, MemoryLocation::Host, true);

            uint8_t* tempStagingData = transferBuffer.map();
            std::memcpy(tempStagingData, newData.data, newData.size);
            transferBuffer.unMap();

            View<Queue> queue = m_device->getQueue(QueueType::Graphics);
            queue->oneShot([this, &transferBuffer, &newData](CommandBuffer& commands) {
                commands.copy(transferBuffer, *this, newData.size);
            });
        }
    }

} // namespace vzt
