#include "vzt/vulkan/uniform.hpp"

namespace vzt
{
    template <class Type>
    UniformBuffer UniformBuffer::From(View<vzt::Device> device, CSpan<Type> data, uint32_t frameNb, bool mappable)
    {
        UniformBuffer result = UniformBuffer(device, data.size * sizeof(Type), frameNb, mappable);

        // Duplicate data per frame
        std::vector<uint8_t> fullData = std::vector<uint8_t>();
        fullData.resize(frameNb * result.m_alignmentByteNb);

        for (uint32_t f = 0; f < frameNb; f++)
            std::memcpy(&fullData[result.m_alignmentByteNb * f], data.data, result.m_perFrameByteNb);

        result.set(fullData);
        return result;
    }

    template <class Type>
    UniformBuffer UniformBuffer::Typed(View<vzt::Device> device, uint32_t frameNb, bool mappable)
    {
        return {device, sizeof(Type), frameNb, mappable};
    }

    template <class Type>
    void UniformBuffer::set(CSpan<Type> data, uint32_t frame)
    {
        uint8_t* ptr = m_transferBuffer.map();
        std::memcpy(ptr + m_alignmentByteNb * frame, data.data, data.size * sizeof(Type));
        m_transferBuffer.unMap();

        // Transfer data
        View<Device>      device = m_buffer.getDevice();
        const View<Queue> queue  = device->getQueue(QueueType::Transfer);
        queue->oneShot([&](CommandBuffer& commands) { //
            commands.copy(m_transferBuffer, m_buffer, data.size * sizeof(Type), m_alignmentByteNb * frame,
                          m_alignmentByteNb * frame);
        });
    }

    template <class Type>
    void UniformBuffer::set(const Type& data, uint32_t frame)
    {
        uint8_t* ptr = m_transferBuffer.map();
        std::memcpy(ptr + m_alignmentByteNb * frame, &data, sizeof(Type));
        m_transferBuffer.unMap();

        View<Device>      device = m_buffer.getDevice();
        const View<Queue> queue  = device->getQueue(QueueType::Transfer);
        queue->oneShot([&](CommandBuffer& commands) { //
            commands.copy(m_transferBuffer, m_buffer, data.size * sizeof(Type), m_alignmentByteNb * frame,
                          m_alignmentByteNb * frame);
        });
    }

    template <class Type>
    void UniformBuffer::write(CommandBuffer& commands, CSpan<Type> data, uint32_t frame)
    {
        uint8_t* ptr = m_transferBuffer.map();
        std::memcpy(ptr + m_alignmentByteNb * frame, data.data, data.size * sizeof(Type));
        m_transferBuffer.unMap();

        commands.copy(m_transferBuffer, m_buffer, data.size * sizeof(Type), m_alignmentByteNb * frame,
                      frame * m_alignmentByteNb);
    }

    template <class Type>
    void UniformBuffer::write(CommandBuffer& commands, const Type& data, uint32_t frame)
    {
        uint8_t* ptr = m_transferBuffer.map();
        std::memcpy(ptr + m_alignmentByteNb * frame, &data, sizeof(Type));
        m_transferBuffer.unMap();

        commands.copy(m_transferBuffer, m_buffer, sizeof(Type), m_alignmentByteNb * frame, frame * m_alignmentByteNb);
    }
} // namespace vzt