#include "vzt/Buffer.hpp"

namespace vzt
{
    template <class Type>
    Buffer Buffer::fromData(View<Device> device, Span<Type> data, BufferUsage usage, MemoryLocation location,
                            bool mappable)
    {
        Buffer buffer{device, data.size * sizeof(Type), usage, location, mappable};
        buffer.update(data);

        return buffer;
    }

    template <class Type>
    Buffer Buffer::fromData(View<Device> device, Span<const Type> data, BufferUsage usage, MemoryLocation location,
                            bool mappable)
    {
        Buffer buffer{device, data.size * sizeof(Type), usage, location, mappable};
        buffer.update(data);

        return buffer;
    }

    template <class Type>
    void Buffer::update(const Span<Type> newData, const std::size_t offset)
    {
        Span<uint8_t> translated{reinterpret_cast<const uint8_t*>(newData.data), newData.size, newData.offset};
        update(translated, offset);
    }

    template <class Type>
    void Buffer::update(const Span<const Type> newData, const std::size_t offset)
    {
        Span<const uint8_t> translated{reinterpret_cast<const uint8_t*>(newData.data), newData.size, newData.offset};
        update(translated, offset);
    }

    MemoryLocation Buffer::getLocation() const { return m_location; }
    VkBuffer       Buffer::getHandle() const { return m_handle; }
} // namespace vzt
