#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	Buffer::Buffer(vzt::Device* device, const std::size_t size, uint8_t* data, VkBufferUsageFlags usage,
	               MemoryUsage memoryUsage)
	    : m_device(device)
	{
		Create(size, data, usage, memoryUsage);
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocation, other.m_allocation);
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocation, other.m_allocation);

		return *this;
	}

	Buffer::~Buffer()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->AllocatorHandle(), m_vkHandle, m_allocation);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

	void Buffer::Update(const std::size_t size, const uint8_t* const newData)
	{
		void* data = nullptr;
		vmaMapMemory(m_device->AllocatorHandle(), m_allocation, &data);
		std::memcpy(data, newData, size);
		vmaUnmapMemory(m_device->AllocatorHandle(), m_allocation);
	}

	void Buffer::Update(const std::size_t size, const std::size_t offset, const uint8_t* const newData)
	{
		uint8_t* data = nullptr;
		vmaMapMemory(m_device->AllocatorHandle(), m_allocation, reinterpret_cast<void**>(&data));
		std::memcpy(data + offset, newData, size);
		vmaUnmapMemory(m_device->AllocatorHandle(), m_allocation);
	}

	VkMemoryRequirements Buffer::MemoryRequirements() const
	{
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_device->VkHandle(), m_vkHandle, &memoryRequirements);
		return memoryRequirements;
	}

	void Buffer::Create(const std::size_t size, const uint8_t* const data, VkBufferUsageFlags usage,
	                    MemoryUsage memoryUsage)
	{
		VkDeviceSize bufferSize = size;

		if (memoryUsage == MemoryUsage::GPU_ONLY)
		{
			VkBuffer      stagingBuffer      = VK_NULL_HANDLE;
			VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;

			stagingBuffer = m_device->CreateBuffer(stagingBufferAlloc, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			                                       VMA_MEMORY_USAGE_CPU_ONLY);

			void* tempstagingData;
			vmaMapMemory(m_device->AllocatorHandle(), stagingBufferAlloc, &tempstagingData);
			memcpy(tempstagingData, data, size);
			vmaUnmapMemory(m_device->AllocatorHandle(), stagingBufferAlloc);

			m_vkHandle = m_device->CreateBuffer(m_allocation, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
			                                    static_cast<VmaMemoryUsage>(vzt::ToUnderlying(memoryUsage)));

			m_device->CopyBuffer(stagingBuffer, m_vkHandle, bufferSize);

			vmaDestroyBuffer(m_device->AllocatorHandle(), stagingBuffer, stagingBufferAlloc);
		}
		else if (memoryUsage == MemoryUsage::CPU_TO_GPU)
		{
			m_vkHandle = m_device->CreateBuffer(m_allocation, bufferSize, usage,
			                                    static_cast<VmaMemoryUsage>(vzt::ToUnderlying(memoryUsage)));

			void* tempstagingData;
			vmaMapMemory(m_device->AllocatorHandle(), m_allocation, &tempstagingData);
			memcpy(tempstagingData, data, size);
			vmaUnmapMemory(m_device->AllocatorHandle(), m_allocation);
		}
		else
		{
			throw std::runtime_error("Not currently implemented...");
		}
	}

} // namespace vzt
