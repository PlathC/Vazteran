#include "vzt/Vulkan/Buffer.hpp"

#include "vzt/Core/Assert.hpp"
#include "vzt/Vulkan/Command.hpp"
#include "vzt/Vulkan/Device.hpp"

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

    Buffer Buffer::From(View<Device> device, CSpan<uint8_t> data, BufferUsage usages, MemoryLocation location,
                        bool mappable)
    {
        VZT_ASSERT(data.size > 0);
        if (mappable)
        {
            Buffer   buffer     = {device, data.size, usages, location, mappable};
            uint8_t* bufferData = buffer.map();
            std::memcpy(bufferData, data.data, data.size);
            buffer.unMap();

            return buffer;
        }

        Buffer buffer         = {device, data.size, usages | BufferUsage::TransferDst, location, mappable};
        Buffer transferBuffer = {device, data.size, BufferUsage::TransferSrc, MemoryLocation::Host, true};
        {
            uint8_t* tempStagingData = transferBuffer.map();
            std::memcpy(tempStagingData, data.data, data.size);
            transferBuffer.unMap();
        }

        const View<Queue> queue = device->getQueue(QueueType::Transfer);
        queue->oneShot([&transferBuffer, &buffer, &data](CommandBuffer& commands) {
            commands.copy(transferBuffer, buffer, data.size);
        });

        return buffer;
    }

    Buffer::Buffer(View<Device> device, std::size_t byteNb, BufferUsage usages, MemoryLocation location, bool mappable)
        : DeviceObject<VkBuffer>(device), m_size(byteNb), m_location(location), m_usages(usages), m_mappable(mappable)
    {
        VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size               = m_size;
        bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage              = toVulkan(m_usages);
        if (!mappable)
            bufferInfo.usage |= toVulkan(BufferUsage::TransferDst);

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags                   = toVma(mappable);
        allocInfo.preferredFlags          = toVma(location);

        vkCheck(vmaCreateBuffer(m_device->getAllocator(), &bufferInfo, &allocInfo, &m_handle, &m_allocation, nullptr),
                "Failed to create vertex buffer!");
    }

    Buffer::Buffer(Buffer&& other) noexcept : DeviceObject<VkBuffer>(std::move(other))
    {
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_size, other.m_size);
        std::swap(m_location, other.m_location);
        std::swap(m_usages, other.m_usages);
        std::swap(m_mappable, other.m_mappable);
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_size, other.m_size);
        std::swap(m_location, other.m_location);
        std::swap(m_usages, other.m_usages);
        std::swap(m_mappable, other.m_mappable);

        DeviceObject<VkBuffer>::operator=(std::move(other));
        return *this;
    }

    Buffer::~Buffer()
    {
        if (m_handle != VK_NULL_HANDLE)
            vmaDestroyBuffer(m_device->getAllocator(), m_handle, m_allocation);
    }

    uint8_t* Buffer::map() const
    {
        uint8_t* data = nullptr;
        vmaMapMemory(m_device->getAllocator(), m_allocation, reinterpret_cast<void**>(&data));

        return data;
    }

    void Buffer::unMap() const { vmaUnmapMemory(m_device->getAllocator(), m_allocation); }

    uint64_t Buffer::getDeviceAddress() const
    {
        VkBufferDeviceAddressInfoKHR info{};
        info.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.buffer = m_handle;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        return table.vkGetBufferDeviceAddress(m_device->getHandle(), &info);
    }
} // namespace vzt
