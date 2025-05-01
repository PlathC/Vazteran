#include "vzt/Vulkan/Buffer.hpp"

namespace vzt
{
    template <class Type>
    Buffer Buffer::From(View<Device> device, CSpan<Type> data, BufferUsage usages, MemoryLocation location,
                        bool mappable)
    {
        const CSpan<uint8_t> translated = {reinterpret_cast<const uint8_t*>(data.data), data.size * sizeof(Type)};
        return From(device, translated, usages, location, mappable);
    }

    inline bool           Buffer::isMappable() const { return m_mappable; }
    inline std::size_t    Buffer::size() const { return m_size; }
    inline MemoryLocation Buffer::getLocation() const { return m_location; }
} // namespace vzt
