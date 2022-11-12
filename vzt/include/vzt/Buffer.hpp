#ifndef VZT_BUFFER_HPP
#define VZT_BUFFER_HPP

#include <functional>

#include <vk_mem_alloc.h>

#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"

namespace vzt
{
    class Device;

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
        static Buffer fromData(View<Device> device, Span<Type> data, BufferUsage usage,
                               MemoryLocation location = MemoryLocation::Device, bool mappable = false);
        template <class Type>
        static Buffer fromData(View<Device> device, CSpan<Type> data, BufferUsage usage,
                               MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer() = default;
        Buffer(View<Device> device, std::size_t byteNb, BufferUsage usage,
               MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        ~Buffer();

        template <class Type>
        void update(CSpan<Type> newData, const std::size_t offset = 0);
        void update(CSpan<uint8_t> newData, const std::size_t offset = 0);

        uint8_t* map();
        void     unMap();

        inline MemoryLocation getLocation() const;
        inline VkBuffer       getHandle() const;

      private:
        View<Device>  m_device{};
        VkBuffer      m_handle     = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        std::size_t    m_size;
        MemoryLocation m_location;
        BufferUsage    m_usage;
        bool           m_mappable = false;
    };

    using BufferSpan = OffsetSpan<Buffer>;

} // namespace vzt

#include "vzt/Buffer.inl"

#endif // VZT_BUFFER_HPP
