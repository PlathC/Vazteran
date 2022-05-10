#ifndef VAZTERAN_VULKAN_BUFFER_HPP
#define VAZTERAN_VULKAN_BUFFER_HPP

#include <vector>

#include <vk_mem_alloc.h>

#include "Vazteran/Core/Macro.hpp"

namespace vzt
{
	class Device;

	enum class MemoryUsage : uint8_t
	{
		PreferDevice = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		PreferHost   = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		Auto         = VMA_MEMORY_USAGE_AUTO
	};
	TO_VULKAN_FUNCTION(MemoryUsage, VmaMemoryUsage)

	enum class BufferUsage : uint32_t
	{
		TransferSrc        = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		TransferDst        = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		UniformTexelBuffer = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
		StorageTexelBuffer = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
		UniformBuffer      = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		StorageBuffer      = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		IndexBuffer        = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VertexBuffer       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		IndirectBuffer     = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,

		// Provided by VK_VERSION_1_2
		ShaderDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,

		// Provided by VK_EXT_transform_feedback
		TransformFeedback = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
		// Provided by VK_EXT_transform_feedback
		TransformFeedbackCounter = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
		// Provided by VK_EXT_conditional_rendering
		ConditionalRendering = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT,
		// Provided by VK_KHR_acceleration_structure
		AccelerationStructureBuildInputReadOnly = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		// Provided by VK_KHR_acceleration_structure
		AccelerationStructureStorage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		// Provided by VK_KHR_ray_tracing_pipeline
		ShaderBindingTable = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,

		// Provided by VK_NV_ray_tracing
		Raytracing = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
		// Provided by VK_EXT_buffer_device_address
		ShaderDeviceAddressExt = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT,
		// Provided by VK_KHR_buffer_device_address
		ShaderDeviceAddressKHR = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
	};
	BITWISE_FUNCTION(BufferUsage)
	TO_VULKAN_FUNCTION(BufferUsage, VkBufferUsageFlagBits)

	class Buffer
	{
	  public:
		Buffer() = default;

		template <class Type>
		Buffer(const vzt::Device* device, const std::vector<Type>& data, BufferUsage usage,
		       MemoryUsage memoryUsage = MemoryUsage::Auto, bool mappable = false, bool persistent = false);
		Buffer(const vzt::Device* device, const std::size_t size, const uint8_t* data, BufferUsage usage,
		       MemoryUsage memoryUsage = MemoryUsage::Auto, bool mappable = false, bool persistent = false);

		Buffer(const Buffer&)            = delete;
		Buffer& operator=(const Buffer&) = delete;

		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(Buffer&& other) noexcept;

		~Buffer();

		template <class Type>
		void update(const std::vector<Type>& newData);
		void update(const std::size_t size, const uint8_t* const newData) const;
		void update(const std::size_t size, const std::size_t offset, const uint8_t* const newData);

		VkBuffer vkHandle() const { return m_vkHandle; }

	  private:
		void create(const std::size_t size, const uint8_t* const data, BufferUsage usage, MemoryUsage memoryUsage,
		            bool mappable, bool persistent);

	  private:
		const vzt::Device* m_device     = nullptr;
		VkBuffer           m_vkHandle   = VK_NULL_HANDLE;
		VmaAllocation      m_allocation = VK_NULL_HANDLE;

		bool m_mappable   = false;
		bool m_persistent = false;
	};
} // namespace vzt

#include "Vazteran/Backend/Vulkan/Buffer.inl"

#endif // VAZTERAN_VULKAN_BUFFER_HPP
