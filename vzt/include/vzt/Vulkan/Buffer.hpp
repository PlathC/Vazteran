#ifndef VZT_VULKAN_BUFFER_HPP
#define VZT_VULKAN_BUFFER_HPP

#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Core.hpp"
#include "vzt/Vulkan/DeviceObject.hpp"

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
    VZT_DEFINE_TO_VULKAN_FUNCTION(BufferUsage, VkBufferUsageFlagBits)

    enum class MemoryLocation
    {
        Host,
        Device
    };

    class Buffer : public DeviceObject<VkBuffer>
    {
      public:
        template <class Type>
        static Buffer fromData(View<Device> device, CSpan<Type> data, BufferUsage usages,
                               MemoryLocation location = MemoryLocation::Device, bool mappable = false);
        static Buffer fromData(View<Device> device, CSpan<uint8_t> data, BufferUsage usages,
                               MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer() = default;
        Buffer(View<Device> device, std::size_t byteNb, BufferUsage usages,
               MemoryLocation location = MemoryLocation::Device, bool mappable = false);

        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        ~Buffer() override;

        uint8_t* map() const;
        void     unMap() const;

        // Requires dext::BufferDeviceAddress
        uint64_t getDeviceAddress() const;

        inline bool           isMappable() const;
        inline std::size_t    size() const;
        inline MemoryLocation getLocation() const;

      private:
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        std::size_t    m_size;
        MemoryLocation m_location;
        BufferUsage    m_usages;
        bool           m_mappable = false;
    };

    struct BufferSpan
    {
        BufferSpan() = default;
        BufferSpan(Buffer& buffer) : buffer(&buffer), size(buffer.size()), offset(0) {}
        BufferSpan(Buffer* buffer) : buffer(buffer), size(buffer->size()), offset(0) {}
        BufferSpan(Buffer& buffer, std::size_t size, std::size_t offset = 0)
            : buffer(&buffer), size(size), offset(offset)
        {
        }
        BufferSpan(Buffer* buffer, std::size_t size, std::size_t offset = 0)
            : buffer(buffer), size(size), offset(offset)
        {
        }

        Buffer*     buffer = nullptr;
        std::size_t size   = 0;
        std::size_t offset = 0;
    };

    struct BufferCSpan
    {
        BufferCSpan() = default;
        BufferCSpan(View<Buffer> bufferView, std::size_t size, std::size_t offset = 0)
            : buffer(bufferView.get()), size(size), offset(offset)
        {
        }
        BufferCSpan(BufferSpan span) : buffer(span.buffer), size(span.size), offset(span.offset) {}
        BufferCSpan(const Buffer& buffer) : buffer(&buffer), size(buffer.size()), offset(0) {}
        BufferCSpan(const Buffer* buffer) : buffer(buffer), size(buffer->size()), offset(0) {}

        const Buffer* buffer = nullptr;
        std::size_t   size   = 0;
        std::size_t   offset = 0;
    };
} // namespace vzt

#include "vzt/Vulkan/Buffer.inl"

#endif // VZT_VULKAN_BUFFER_HPP
