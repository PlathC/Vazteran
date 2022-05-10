#include <cassert>
#include <stdexcept>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"

namespace vzt
{
	Buffer::Buffer(const vzt::Device* device, const std::size_t size, const uint8_t* data, BufferUsage usage,
	               MemoryUsage memoryUsage, bool mappable, bool persistent)
	    : m_device(device), m_mappable(mappable), m_persistent(persistent)
	{
		create(size, data, usage, memoryUsage, mappable, persistent);
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocation, other.m_allocation);
		std::swap(m_mappable, other.m_mappable);
		std::swap(m_persistent, other.m_persistent);
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocation, other.m_allocation);
		std::swap(m_mappable, other.m_mappable);
		std::swap(m_persistent, other.m_persistent);

		return *this;
	}

	Buffer::~Buffer()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(m_device->getAllocatorHandle(), m_vkHandle, m_allocation);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

	void Buffer::update(const std::size_t size, const uint8_t* const newData) const
	{
		assert(m_mappable && "The buffer must be signaled as mappable before being mapped to CPU data");

		void* data = nullptr;
		vmaMapMemory(m_device->getAllocatorHandle(), m_allocation, &data);
		std::memcpy(data, newData, size);
		vmaUnmapMemory(m_device->getAllocatorHandle(), m_allocation);
	}

	void Buffer::update(const std::size_t size, const std::size_t offset, const uint8_t* const newData)
	{
		assert(m_mappable && "The buffer must be signaled as mappable before being mapped to CPU data");

		uint8_t* data = nullptr;
		vmaMapMemory(m_device->getAllocatorHandle(), m_allocation, reinterpret_cast<void**>(&data));
		std::memcpy(data + offset, newData, size);
		vmaUnmapMemory(m_device->getAllocatorHandle(), m_allocation);
	}

	void Buffer::create(const std::size_t size, const uint8_t* const data, BufferUsage usage, MemoryUsage memoryUsage,
	                    bool mappable, bool persistent)
	{
		const VkDeviceSize             bufferSize = size;
		const VmaAllocationCreateFlags flags = mappable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0;
		if (!mappable)
		{
			VkBuffer      stagingBuffer      = VK_NULL_HANDLE;
			VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;

			stagingBuffer = m_device->createBuffer(
			    stagingBufferAlloc, bufferSize, vzt::toVulkan(BufferUsage::TransferSrc),
			    vzt::toVulkan(MemoryUsage::PreferHost), VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

			void* tempstagingData;
			vmaMapMemory(m_device->getAllocatorHandle(), stagingBufferAlloc, &tempstagingData);
			std::memcpy(tempstagingData, data, size);
			vmaUnmapMemory(m_device->getAllocatorHandle(), stagingBufferAlloc);

			m_vkHandle =
			    m_device->createBuffer(m_allocation, bufferSize, vzt::toVulkan(BufferUsage::TransferDst | usage),
			                           vzt::toVulkan(memoryUsage), flags);

			m_device->copyBuffer(stagingBuffer, m_vkHandle, bufferSize);

			vmaDestroyBuffer(m_device->getAllocatorHandle(), stagingBuffer, stagingBufferAlloc);
		}
		else
		{
			m_vkHandle = m_device->createBuffer(m_allocation, bufferSize, vzt::toVulkan(usage),
			                                    vzt::toVulkan(memoryUsage), flags);

			void* tempstagingData;
			vmaMapMemory(m_device->getAllocatorHandle(), m_allocation, &tempstagingData);
			memcpy(tempstagingData, data, size);
			vmaUnmapMemory(m_device->getAllocatorHandle(), m_allocation);
		}
	}

} // namespace vzt
