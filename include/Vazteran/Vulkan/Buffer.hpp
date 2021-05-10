#ifndef VAZTERAN_BUFFER_HPP
#define VAZTERAN_BUFFER_HPP

#include "Vazteran/Vulkan/GpuObject.hpp"

namespace vzt {
    class LogicalDevice;

    template<class Type>
    class StagedBuffer {
    public:
        StagedBuffer(LogicalDevice* device, const std::vector<Type>& data, VkBufferUsageFlags usage);

        StagedBuffer(StagedBuffer&) = delete;
        StagedBuffer& operator=(StagedBuffer&) = delete;

        StagedBuffer(StagedBuffer&& other) noexcept;
        StagedBuffer& operator=(StagedBuffer&& other) noexcept;

        VkBuffer VkHandle() const { return m_vkHandle; }
        std::size_t Size() const { return m_size; }

        ~StagedBuffer();

    private:
        vzt::LogicalDevice* m_device = nullptr;
        VkBuffer m_vkHandle = VK_NULL_HANDLE;
        VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
        std::size_t m_size = 0;
    };

    using VertexBuffer = vzt::StagedBuffer<vzt::Vertex>;
    using IndexBuffer = vzt::StagedBuffer<uint32_t>;
}

#include "Vazteran/Vulkan/Buffer.inl"

#endif //VAZTERAN_BUFFER_HPP
