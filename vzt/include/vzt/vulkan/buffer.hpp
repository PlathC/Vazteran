#ifndef VZT_VULKAN_BUFFER_HPP
#define VZT_VULKAN_BUFFER_HPP

#include "vzt/core/type.hpp"
#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/setup.hpp"

namespace vzt
{
    class Device;
    class Queue;

    enum class MemoryLocation
    {
        Host,
        Device
    };

    class Buffer : public DeviceObject<VkBuffer>
    {
      public:
        template <class Type>
        static Buffer From(View<Device> device, CSpan<Type> data, BufferUsage usages,
                           MemoryLocation location = MemoryLocation::Device, bool mappable = false);
        static Buffer From(View<Device> device, CSpan<uint8_t> data, BufferUsage usages,
                           MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer() = default;
        Buffer(View<Device> device, std::size_t byteNb, BufferUsage usages,
               MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        ~Buffer() override;

        uint8_t* map() const;
        void     unMap() const;

        // Requires dext::BufferDeviceAddress
        uint64_t getDeviceAddress() const;

        inline bool           isMappable() const;
        inline std::size_t    size() const;
        inline MemoryLocation getLocation() const;

      private:
        VmaAllocation  m_allocation = VK_NULL_HANDLE;
        std::size_t    m_size       = 0;
        MemoryLocation m_location   = MemoryLocation::Host;
        BufferUsage    m_usages     = BufferUsage::None;
        bool           m_mappable   = false;
    };

    struct BufferSpan
    {
        BufferSpan() = default;
        BufferSpan(Buffer& buffer) : buffer(&buffer), size(buffer.size()), offset(0) {}
        BufferSpan(Buffer* buffer) : buffer(buffer), size(buffer->size()), offset(0) {}
        BufferSpan(Buffer& buffer, std::size_t size, std::size_t offset = 0)
            : buffer(&buffer), size(size), offset(offset)
        {
        }
        BufferSpan(Buffer* buffer, std::size_t size, std::size_t offset = 0)
            : buffer(buffer), size(size), offset(offset)
        {
        }

        Buffer*     buffer = nullptr;
        std::size_t size   = 0;
        std::size_t offset = 0;
    };

    struct BufferCSpan
    {
        BufferCSpan() = default;
        BufferCSpan(View<Buffer> bufferView, std::size_t size, std::size_t offset = 0)
            : buffer(bufferView.get()), size(size), offset(offset)
        {
        }
        BufferCSpan(BufferSpan span) : buffer(span.buffer), size(span.size), offset(span.offset) {}
        BufferCSpan(const Buffer& buffer) : buffer(&buffer), size(buffer.size()), offset(0) {}
        BufferCSpan(const Buffer* buffer) : buffer(buffer), size(buffer->size()), offset(0) {}

        const Buffer* buffer = nullptr;
        std::size_t   size   = 0;
        std::size_t   offset = 0;
    };
} // namespace vzt

#include "vzt/vulkan/buffer.inl"

#endif // VZT_VULKAN_BUFFER_HPP
