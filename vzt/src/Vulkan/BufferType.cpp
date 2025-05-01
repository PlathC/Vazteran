#include "vzt/Vulkan/BufferType.hpp"

#include <libcodedirectory.h>

#include "vzt/Core/Assert.hpp"

namespace vzt
{
    UniformBuffer::UniformBuffer(View<Device> device, std::size_t perFrameByteNb, uint32_t frameNb, bool mappable)
        : m_perFrameByteNb(perFrameByteNb), m_frameNb(frameNb)
    {
        const auto hardware = device->getHardware();
        m_alignmentByteNb   = hardware.getUniformAlignment(perFrameByteNb);

        m_buffer = Buffer( //
            device, m_alignmentByteNb * frameNb, BufferUsage::UniformBuffer | BufferUsage::TransferDst,
            MemoryLocation::Device, mappable);

        m_transferBuffer =
            Buffer(device, perFrameByteNb * frameNb, BufferUsage::TransferSrc, MemoryLocation::Host, true);
    }

    uint8_t* UniformBuffer::map(uint32_t frame) const
    {
        VZT_ASSERT(m_buffer.isMappable());
        VZT_ASSERT(frame < m_frameNb);
        uint8_t* data = m_buffer.map();
        return data + frame * m_alignmentByteNb;
    }

    void UniformBuffer::unMap() const { m_buffer.unMap(); }

    [[nodiscard]] BufferSpan UniformBuffer::getSpan(uint32_t frame)
    {
        VZT_ASSERT(frame < m_frameNb);
        return {m_buffer, m_perFrameByteNb, frame * m_alignmentByteNb};
    }

    [[nodiscard]] BufferCSpan UniformBuffer::getSpan(uint32_t frame) const
    {
        VZT_ASSERT(frame < m_frameNb);
        return {m_buffer, m_perFrameByteNb, frame * m_alignmentByteNb};
    }

    DescriptorBuffer UniformBuffer::getDescriptor(uint32_t frame) const
    {
        VZT_ASSERT(frame < m_frameNb);
        const vzt::BufferCSpan span = {m_buffer, m_perFrameByteNb, frame * m_alignmentByteNb};
        return vzt::DescriptorBuffer{vzt::DescriptorType::UniformBuffer, span};
    }

} // namespace vzt