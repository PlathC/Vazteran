#include <cassert>
#include <stdexcept>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"

namespace vzt
{
	BufferSpan::BufferSpan(Buffer& buffer) : parent(&buffer), offset(0), range(parent->m_size) {}

	BufferSpan::BufferSpan(Buffer* buffer, std::size_t offset, std::size_t range)
	    : parent(buffer), offset(offset), range(range)
	{
	}

	Buffer::Buffer(const Device* device, const Span<const uint8_t> bufferData, BufferUsage usage,
	               MemoryUsage memoryUsage, bool mappable, bool persistent)
	    : m_device(device), m_mappable(mappable), m_persistent(persistent), m_size(bufferData.size())
	{
		create(bufferData, usage, memoryUsage, mappable, persistent);
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocation, other.m_allocation);
		std::swap(m_mappable, other.m_mappable);
		std::swap(m_persistent, other.m_persistent);
		std::swap(m_size, other.m_size);
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_allocation, other.m_allocation);
		std::swap(m_mappable, other.m_mappable);
		std::swap(m_persistent, other.m_persistent);
		std::swap(m_size, other.m_size);

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

	void Buffer::update(const Span<const uint8_t> newData, const std::size_t offset)
	{
		assert(m_mappable && "The buffer must be signaled as mappable before being mapped to CPU data");

		uint8_t* data = nullptr;
		vmaMapMemory(m_device->getAllocatorHandle(), m_allocation, reinterpret_cast<void**>(&data));
		std::memcpy(data + offset, newData.data(), newData.size());
		vmaUnmapMemory(m_device->getAllocatorHandle(), m_allocation);
	}

	BufferSpan Buffer::get(std::size_t from, std::size_t to) { return {this, from, to - from}; }

	void Buffer::create(const Span<const uint8_t> bufferData, BufferUsage usage, MemoryUsage memoryUsage, bool mappable,
	                    bool persistent)
	{
		const VkDeviceSize bufferSize = bufferData.size();
		if (!mappable)
		{
			VkBuffer      stagingBuffer      = VK_NULL_HANDLE;
			VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;

			stagingBuffer = m_device->createBuffer(
			    stagingBufferAlloc, bufferSize, vzt::toVulkan(BufferUsage::TransferSrc),
			    vzt::toVulkan(MemoryUsage::PreferHost), VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

			void* tempstagingData;
			vmaMapMemory(m_device->getAllocatorHandle(), stagingBufferAlloc, &tempstagingData);
			std::memcpy(tempstagingData, bufferData.data(), bufferData.size());
			vmaUnmapMemory(m_device->getAllocatorHandle(), stagingBufferAlloc);

			m_vkHandle =
			    m_device->createBuffer(m_allocation, bufferSize, vzt::toVulkan(BufferUsage::TransferDst | usage),
			                           vzt::toVulkan(memoryUsage), 0);

			m_device->copyBuffer(stagingBuffer, m_vkHandle, bufferSize);

			vmaDestroyBuffer(m_device->getAllocatorHandle(), stagingBuffer, stagingBufferAlloc);
		}
		else
		{
			m_vkHandle =
			    m_device->createBuffer(m_allocation, bufferSize, vzt::toVulkan(usage), vzt::toVulkan(memoryUsage),
			                           VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

			void* tempstagingData;
			vmaMapMemory(m_device->getAllocatorHandle(), m_allocation, &tempstagingData);
			memcpy(tempstagingData, bufferData.data(), bufferData.size());
			vmaUnmapMemory(m_device->getAllocatorHandle(), m_allocation);
		}
	}

} // namespace vzt
