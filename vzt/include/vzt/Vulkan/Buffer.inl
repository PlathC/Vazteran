#include "vzt/Vulkan/Buffer.hpp"

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
    Buffer Buffer::fromData(View<Device> device, CSpan<Type> data, BufferUsage usage, MemoryLocation location,
                            bool mappable)
    {
        Buffer buffer{device, data.size * sizeof(Type), usage, location, mappable};
        buffer.update(data);

        return buffer;
    }

    template <class Type>
    void Buffer::update(CSpan<Type> newData, const std::size_t offset)
    {
        const Span<uint8_t> translated{reinterpret_cast<const uint8_t*>(newData.data), newData.size, newData.offset};
        update(translated, offset);
    }

    inline std::size_t    Buffer::size() const { return m_size; }
    inline MemoryLocation Buffer::getLocation() const { return m_location; }
    inline VkBuffer       Buffer::getHandle() const { return m_handle; }
} // namespace vzt
