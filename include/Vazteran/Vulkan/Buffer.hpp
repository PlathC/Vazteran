#ifndef VAZTERAN_BUFFER_HPP
#define VAZTERAN_BUFFER_HPP

#include "Vazteran/Vulkan/GpuObject.hpp"

namespace vzt {
    class LogicalDevice;

    template<class Type>
    class StagedBuffer {
    public:
        StagedBuffer(LogicalDevice* device, const std::vector<Type>& data, VkBufferUsageFlags usage);

        VkBuffer VkHandle() const { return m_vkHandle; }
        std::size_t Size() const { return m_size; }

        ~StagedBuffer();

    private:
        LogicalDevice* m_device;
        VkBuffer m_vkHandle;
        VkDeviceMemory m_bufferMemory;
        std::size_t m_size;
    };

    using VertexBuffer = StagedBuffer<Vertex>;
    using IndexBuffer = StagedBuffer<uint32_t>;
}

#include "Vazteran/Vulkan/Buffer.inl"

#endif //VAZTERAN_BUFFER_HPP
