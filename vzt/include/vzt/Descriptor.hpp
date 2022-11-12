#ifndef VZT_DESCRIPTOR_HPP
#define VZT_DESCRIPTOR_HPP

#include <vulkan/vulkan_core.h>

#include "vzt/Buffer.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"

namespace vzt
{
    class DescriptorPool;
    class Texture;

    enum class DescriptorType
    {
        Sampler              = VK_DESCRIPTOR_TYPE_SAMPLER,
        CombinedSampler      = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        SampledImage         = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        StorageImage         = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        UniformTexelBuffer   = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        StorageTexelBuffer   = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        UniformBuffer        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        StorageBuffer        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        UniformBufferDynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        InputAttachment      = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(DescriptorType, VkDescriptorType)

    class DescriptorLayout
    {
      public:
        DescriptorLayout(View<Device> device);

        DescriptorLayout(const DescriptorLayout& other)            = delete;
        DescriptorLayout& operator=(const DescriptorLayout& other) = delete;

        DescriptorLayout(DescriptorLayout&& other) noexcept;
        DescriptorLayout& operator=(DescriptorLayout&& other) noexcept;

        ~DescriptorLayout();

        void addBinding(uint32_t binding, DescriptorType type);
        void compile();

        inline VkDescriptorSetLayout getHandle() const;

      private:
        View<Device>          m_device{};
        VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;

        using Bindings = std::unordered_map<uint32_t /*binding*/, vzt::DescriptorType /*type */>;
        Bindings m_bindings;
    };

    class DescriptorSet
    {
      public:
        inline VkDescriptorSet getHandle() const;

        friend DescriptorPool;

      private:
        DescriptorSet(VkDescriptorSet handle);

        VkDescriptorSet m_handle = VK_NULL_HANDLE;
    };

    template <class Type>
    using Indexed = std::unordered_map<uint32_t, Type>;
    class DescriptorPool
    {
      public:
        const static std::vector<DescriptorType> DefaultDescriptors;

        DescriptorPool() = default;
        DescriptorPool(View<Device> device, std::vector<DescriptorType> descriptorTypes = {}, uint32_t maxSetNb = 64);

        DescriptorPool(const DescriptorPool&)            = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        DescriptorPool(DescriptorPool&&) noexcept;
        DescriptorPool& operator=(DescriptorPool&&) noexcept;

        ~DescriptorPool();

        void allocate(uint32_t count, const vzt::DescriptorLayout& layout);

        VkDescriptorSet operator[](uint32_t i) const { return m_descriptors[i]; }

        void update(std::size_t i, const Indexed<BufferSpan>& bufferDescriptors,
                    const Indexed<View<Texture>>& imageDescriptors);
        void update(std::size_t i, const Indexed<BufferSpan>& bufferDescriptors);
        void update(std::size_t i, const Indexed<View<Texture>>& imageDescriptors);

        void update(const Indexed<BufferSpan>& bufferDescriptors, const Indexed<View<Texture>>& imageDescriptors);
        void update(const Indexed<BufferSpan>& bufferDescriptors);
        void update(const Indexed<View<Texture>>& imageDescriptors);

        inline uint32_t         getRemaining() const;
        inline uint32_t         getMaxSetNb() const;
        inline VkDescriptorPool getHandle() const;

      private:
        View<Device>                 m_device;
        VkDescriptorPool             m_handle = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptors;

        uint32_t m_maxSetNb = 0;
    };

} // namespace vzt

#include "vzt/Descriptor.inl"

#endif // VZT_DESCRIPTOR_HPP
