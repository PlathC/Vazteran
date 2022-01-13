#include "Vazteran/Framework/Vulkan/Device.hpp"

namespace vzt
{
	template <class Type>
	Buffer<Type>::Buffer(vzt::Device *device, const std::vector<Type> &data, VkBufferUsageFlags usage)
	    : m_device(device), m_size(data.size())
	{

		VkDeviceSize bufferSize = sizeof(data[0]) * data.size();
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;

		m_device->CreateBuffer(
		    stagingBuffer, stagingBufferAlloc, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void *tempstagingData;
		vmaMapMemory(m_device->AllocatorHandle(), stagingBufferAlloc, &tempstagingData);
		memcpy(tempstagingData, data.data(), static_cast<std::size_t>(bufferSize));
		vmaUnmapMemory(m_device->AllocatorHandle(), stagingBufferAlloc);

		m_device->CreateBuffer(
		    m_vkHandle, m_allocation, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VMA_MEMORY_USAGE_GPU_ONLY);

		m_device->CopyBuffer(stagingBuffer, m_vkHandle, bufferSize);

		vmaDestroyBuffer(m_device->AllocatorHandle(), stagingBuffer, stagingBufferAlloc);
	}

	template <class Type>
	Buffer<Type>::Buffer(
	    vzt::Device *device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::size_t size)
	    : m_device(device), m_size(size)
	{
		VkDeviceSize bufferSize = m_size * sizeof(Type);

		m_device->CreateBuffer(m_vkHandle, m_allocation, bufferSize, usage, VMA_MEMORY_USAGE_GPU_ONLY, properties);
	}

	template <class Type> Buffer<Type>::Buffer(Buffer<Type> &&other) noexcept
	{
		m_device = std::exchange(other.m_device, nullptr);
		m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		m_allocation = std::exchange(other.m_allocation, static_cast<decltype(m_allocation)>(VK_NULL_HANDLE));
		m_size = std::exchange(other.m_size, 0);
	}

	template <class Type> Buffer<Type> &Buffer<Type>::operator=(Buffer<Type> &&other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocation, other.m_allocation);
		std::swap(m_size, other.m_size);
	}

	template <class Type> template <class SetType> void Buffer<Type>::Update(const std::vector<SetType> &newData)
	{
		VkDeviceSize bufferSize = sizeof(newData[0]) * newData.size();
		m_size = newData.size();

		void *data;
		vmaMapMemory(m_device->AllocatorHandle(), m_allocation, &data);
		memcpy(data, newData.data(), static_cast<std::size_t>(bufferSize));
		vmaUnmapMemory(m_device->AllocatorHandle(), m_allocation);
	}

	template <class Type> Buffer<Type>::~Buffer()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->AllocatorHandle(), m_vkHandle, m_allocation);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

} // namespace vzt
