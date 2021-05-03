#include "Vazteran/Vulkan/LogicalDevice.hpp"

namespace vzt {
    template<class Type>
    StagedBuffer<Type>::StagedBuffer(vzt::LogicalDevice* device, const std::vector<Type>& data, VkBufferUsageFlags usage) :
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
            memcpy(dataPtr, data.data(), static_cast<size_t>(bufferSize));
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
    StagedBuffer<Type>::~StagedBuffer() {
        vkDestroyBuffer(m_device->VkHandle(), m_vkHandle, nullptr);
        vkFreeMemory(m_device->VkHandle(), m_bufferMemory, nullptr);
    }

}