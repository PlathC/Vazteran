#include "Vazteran/Vulkan/LogicalDevice.hpp"

namespace vzt {
    template<class Type>
    Buffer<Type>::Buffer(vzt::LogicalDevice* device, const std::vector<Type>& data, VkBufferUsageFlags usage) :
            m_device(device), m_size(data.size()) {

        VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_device->CreateBuffer(
                stagingBuffer, stagingBufferMemory, bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        void* dataPtr;
        vkMapMemory(m_device->VkHandle(), stagingBufferMemory, 0, bufferSize, 0, &dataPtr);
            memcpy(dataPtr, data.data(), static_cast<std::size_t>(bufferSize));
        vkUnmapMemory(m_device->VkHandle(), stagingBufferMemory);

        m_device->CreateBuffer(
                m_vkHandle, m_bufferMemory, bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_device->CopyBuffer(stagingBuffer, m_vkHandle, bufferSize);

        vkDestroyBuffer(m_device->VkHandle(), stagingBuffer, nullptr);
        vkFreeMemory(m_device->VkHandle(), stagingBufferMemory, nullptr);
    }

    template<class Type>
    Buffer<Type>::Buffer(vzt::LogicalDevice* device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                         std::size_t size):
            m_device(device), m_size(size) {
        VkDeviceSize bufferSize = m_size * sizeof(Type);
        m_device->CreateBuffer( m_vkHandle, m_bufferMemory, bufferSize, usage, properties);
    }

    template<class Type>
    Buffer<Type>::Buffer(Buffer<Type>&& other) noexcept {
        m_device = std::exchange(other.m_device, nullptr);
        m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
        m_bufferMemory = std::exchange(other.m_bufferMemory, static_cast<decltype(m_bufferMemory)>(VK_NULL_HANDLE));
        m_size = std::exchange(other.m_size, 0);
    }

    template<class Type>
    Buffer<Type>& Buffer<Type>::operator=(Buffer<Type>&& other) noexcept {
        std::swap(m_device, other.m_device);
        std::swap(m_vkHandle, other.m_vkHandle);
        std::swap(m_bufferMemory, other.m_bufferMemory);
        std::swap(m_size, other.m_size);
    }

    template<class Type>
    template<class SetType>
    void Buffer<Type>::Update(const std::vector<SetType>& newData) {
        VkDeviceSize bufferSize = sizeof(newData[0]) * newData.size();
        m_size = newData.size();

        void* data;
        vkMapMemory(m_device->VkHandle(), m_bufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, newData.data(), static_cast<std::size_t>(bufferSize));
        vkUnmapMemory(m_device->VkHandle(), m_bufferMemory);
    }

    template<class Type>
    Buffer<Type>::~Buffer() {
        if (m_vkHandle != VK_NULL_HANDLE)
            vkDestroyBuffer(m_device->VkHandle(), m_vkHandle, nullptr);

        if (m_bufferMemory != VK_NULL_HANDLE)
            vkFreeMemory(m_device->VkHandle(), m_bufferMemory, nullptr);
    }

}