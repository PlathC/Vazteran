#include "vzt/Vulkan/Buffer.hpp"

namespace vzt
{
    template <class Type>
    Buffer Buffer::fromData(View<Device> device, OffsetCSpan<Type> data, BufferUsage usages, MemoryLocation location,
                            bool mappable)
    {
        const OffsetCSpan<uint8_t> translated = {reinterpret_cast<const uint8_t*>(data.data), data.size * sizeof(Type),
                                                 data.offset};
        return fromData(device, translated, usages, location, mappable);
    }

    inline bool           Buffer::isMappable() const { return m_mappable; }
    inline std::size_t    Buffer::size() const { return m_size; }
    inline MemoryLocation Buffer::getLocation() const { return m_location; }
    inline VkBuffer       Buffer::getHandle() const { return m_handle; }
} // namespace vzt
