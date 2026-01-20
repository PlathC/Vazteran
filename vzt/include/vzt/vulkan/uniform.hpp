#ifndef VZT_VULKAN_BUFFER_TYPE_HPP
#define VZT_VULKAN_BUFFER_TYPE_HPP

#include "vzt/vulkan/command.hpp"
#include "vzt/vulkan/descriptor.hpp"
#include "vzt/vulkan/device.hpp"

namespace vzt
{
    class UniformBuffer
    {
      public:
        template <class Type>
        static UniformBuffer From(View<Device> device, CSpan<Type> data, uint32_t frameNb, bool mappable = false);
        template <class Type>
        static UniformBuffer Typed(View<vzt::Device> device, uint32_t frameNb, bool mappable = false);

        UniformBuffer() = default;
        UniformBuffer(View<Device> device, std::size_t perFrameByteNb, uint32_t frameNb, bool mappable = false);

        UniformBuffer(const UniformBuffer& buffer)            = delete;
        UniformBuffer& operator=(const UniformBuffer& buffer) = delete;

        UniformBuffer(UniformBuffer&& buffer)            = default;
        UniformBuffer& operator=(UniformBuffer&& buffer) = default;

        uint8_t* map(uint32_t frameNb = 0) const;
        void     unMap() const;

        template <class Type>
        void set(CSpan<Type> data, uint32_t frame = 0);

        template <class Type>
        void set(const Type& data, uint32_t frame = 0);

        template <class Type>
        void write(CommandBuffer& commands, CSpan<Type> data, uint32_t frame);

        template <class Type>
        void write(CommandBuffer& commands, const Type& data, uint32_t frame);

        [[nodiscard]] BufferSpan       getSpan(uint32_t frame = 0);
        [[nodiscard]] BufferCSpan      getSpan(uint32_t frame = 0) const;
        [[nodiscard]] DescriptorBuffer getDescriptor(uint32_t frame = 0) const;

      private:
        std::size_t m_perFrameByteNb  = 0;
        std::size_t m_frameNb         = 0;
        std::size_t m_alignmentByteNb = 0;

        Buffer m_buffer;
        Buffer m_transferBuffer;
    };
} // namespace vzt

#include "vzt/vulkan/uniform.inl"

#endif