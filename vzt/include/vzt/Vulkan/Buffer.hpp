#ifndef VZT_VULKAN_BUFFER_HPP
#define VZT_VULKAN_BUFFER_HPP

#include <functional>

#include <vk_mem_alloc.h>

#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"

namespace vzt
{
    class Device;
    class Queue;

    enum class BufferUsage : uint32_t
    {
        None               = 0,
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
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(BufferUsage)
    VZT_DEFINE_TO_VULKAN_FUNCTION(BufferUsage, VkBufferUsageFlags)

    enum class MemoryLocation
    {
        Host,
        Device
    };

    class Buffer
    {
      public:
        template <class Type>
        static Buffer fromData(View<Device> device, OffsetCSpan<Type> data, BufferUsage usages,
                               MemoryLocation location = MemoryLocation::Device, bool mappable = false);
        static Buffer fromData(View<Device> device, OffsetCSpan<uint8_t> data, BufferUsage usages,
                               MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer() = default;
        Buffer(View<Device> device, std::size_t byteNb, BufferUsage usages,
               MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        ~Buffer();

        uint8_t* map();
        void     unMap();

        // Requires dext::BufferDeviceAddress
        uint64_t getDeviceAddress() const;

        inline bool           isMappable() const;
        inline std::size_t    size() const;
        inline MemoryLocation getLocation() const;
        inline VkBuffer       getHandle() const;

      private:
        View<Device>  m_device{};
        VkBuffer      m_handle     = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        std::size_t    m_size;
        MemoryLocation m_location;
        BufferUsage    m_usages;
        bool           m_mappable = false;
    };

    using BufferSpan  = OffsetSpan<Buffer>;
    using BufferCSpan = OffsetCSpan<Buffer>;

} // namespace vzt

#include "vzt/Vulkan/Buffer.inl"

#endif // VZT_VULKAN_BUFFER_HPP
