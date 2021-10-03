#ifndef VAZTERAN_BUFFER_HPP
#define VAZTERAN_BUFFER_HPP

#include <cctype>

#include <vk_mem_alloc.h>

#include "Vazteran/Data/Vertex.hpp"

namespace vzt {
    class LogicalDevice;

    template<class Type>
    class Buffer {
    public:
        Buffer(LogicalDevice* device, const std::vector<Type>& data, VkBufferUsageFlags usage);
        Buffer(vzt::LogicalDevice* device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
               std::size_t size = 1);

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        template<class SetType>
        void Update(const std::vector<SetType>& newData);

        VkBuffer VkHandle() const { return m_vkHandle; }
        std::size_t Size() const { return m_size; }

        ~Buffer();

    private:
        vzt::LogicalDevice* m_device = nullptr;
        VkBuffer m_vkHandle = VK_NULL_HANDLE;
        VmaAllocation m_allocation;
        std::size_t m_size = 0;
    };

    using VertexBuffer = Buffer<vzt::Vertex>;
    using IndexBuffer = Buffer<uint32_t>;
}

#include "Vazteran/Vulkan/Buffer.inl"

#endif //VAZTERAN_BUFFER_HPP
