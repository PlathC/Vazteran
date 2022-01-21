#ifndef VAZTERAN_BACKEND_VULKAN_BUFFER_HPP
#define VAZTERAN_BACKEND_VULKAN_BUFFER_HPP

#include <vector>

#include <vk_mem_alloc.h>

namespace vzt
{
	class Device;

	enum class MemoryUsage : uint8_t
	{
		GPU_ONLY   = VMA_MEMORY_USAGE_GPU_ONLY,
		CPU_TO_GPU = VMA_MEMORY_USAGE_CPU_TO_GPU,
	};

	class Buffer
	{
	  public:
		Buffer() = default;

		template <class Type>
		Buffer(vzt::Device* device, const std::vector<Type>& data, VkBufferUsageFlags usage,
		       MemoryUsage memoryUsage = MemoryUsage::GPU_ONLY);
		Buffer(vzt::Device* device, const std::size_t size, uint8_t* data, VkBufferUsageFlags usage,
		       MemoryUsage memoryUsage = MemoryUsage::GPU_ONLY);

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(Buffer&& other) noexcept;

		~Buffer();

		template <class Type>
		void Update(const std::vector<Type>& newData);
		void Update(const std::size_t size, const uint8_t* const newData);

		void Update(const std::size_t size, const std::size_t offset, const uint8_t* const newData);

		VkMemoryRequirements MemoryRequirements() const;

		VkBuffer VkHandle() const { return m_vkHandle; }

	  private:
		void Create(const std::size_t size, const uint8_t* const data, VkBufferUsageFlags usage,
		            MemoryUsage memoryUsage);

	  private:
		vzt::Device*  m_device     = nullptr;
		VkBuffer      m_vkHandle   = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
	};
} // namespace vzt

#include "Vazteran/Backend/Vulkan/Buffer.inl"

#endif // VAZTERAN_BACKEND_VULKAN_BUFFER_HPP
