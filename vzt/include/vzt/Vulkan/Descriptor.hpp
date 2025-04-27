#ifndef VZT_VULKAN_DESCRIPTOR_HPP
#define VZT_VULKAN_DESCRIPTOR_HPP

#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/Image.hpp"
#include "vzt/Vulkan/Texture.hpp"

namespace vzt
{
    class DescriptorPool;
    class ImageView;
    class AccelerationStructure;

    enum class DescriptorType
    {
        Sampler               = VK_DESCRIPTOR_TYPE_SAMPLER,
        CombinedSampler       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        SampledImage          = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        StorageImage          = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        UniformTexelBuffer    = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        StorageTexelBuffer    = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        UniformBuffer         = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        StorageBuffer         = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        UniformBufferDynamic  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        StorageBufferDynamic  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        InputAttachment       = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        InlineUniformBlock    = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
        AccelerationStructure = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        None                  = VK_DESCRIPTOR_TYPE_MAX_ENUM
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(DescriptorType, VkDescriptorType)

    class DescriptorLayout
    {
      public:
        DescriptorLayout() = default;
        DescriptorLayout(View<Device> device);

        DescriptorLayout(const DescriptorLayout& other);
        DescriptorLayout& operator=(const DescriptorLayout& other);

        DescriptorLayout(DescriptorLayout&& other) noexcept;
        DescriptorLayout& operator=(DescriptorLayout&& other) noexcept;

        ~DescriptorLayout();

        void addBinding(uint32_t binding, DescriptorType type);
        void compile();

        using Bindings = std::unordered_map<uint32_t /*binding*/, DescriptorType /*type */>;
        inline Bindings&             getBindings();
        inline const Bindings&       getBindings() const;
        inline uint32_t              size() const;
        inline VkDescriptorSetLayout getHandle() const;

      private:
        View<Device>          m_device{};
        VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;

        Bindings m_bindings;
        bool     m_compiled = false;
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

    struct DescriptorBuffer
    {
        DescriptorType type;
        BufferCSpan    buffer;
    };

    struct DescriptorImage
    {
        DescriptorType  type;
        View<ImageView> image;
        View<Sampler>   sampler; // Only for DescriptorType::SampledImage
        ImageLayout     layout = ImageLayout::ShaderReadOnlyOptimal;
    };

    struct DescriptorAccelerationStructure
    {
        DescriptorType              type;
        View<AccelerationStructure> accelerationStructure;
    };

    class Pipeline;
    using DescriptorWrite   = std::variant<DescriptorBuffer, DescriptorImage, DescriptorAccelerationStructure>;
    using IndexedDescriptor = std::unordered_map<uint32_t, DescriptorWrite>;
    class DescriptorPool : public DeviceObject<VkDescriptorPool>
    {
      public:
        const static std::vector<DescriptorType> DefaultDescriptors;

        DescriptorPool() = default;
        DescriptorPool(View<Device> device, std::unordered_set<DescriptorType> descriptorTypes = {},
                       uint32_t maxSetNb = 64, uint32_t maxPerTypeNb = 64);
        DescriptorPool(View<Device> device, const DescriptorLayout& descriptorLayout, uint32_t maxSetNb = 64);
        DescriptorPool(View<Device> device, const Pipeline& descriptorLayout, uint32_t count = 64);

        DescriptorPool(const DescriptorPool&)            = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        DescriptorPool(DescriptorPool&&) noexcept;
        DescriptorPool& operator=(DescriptorPool&&) noexcept;

        ~DescriptorPool() override;

        void                 allocate(uint32_t count, const DescriptorLayout& layout);
        inline DescriptorSet operator[](uint32_t i) const;

        void update(std::size_t descriptorId, const IndexedDescriptor& descriptors);
        void update(const IndexedDescriptor& descriptors);

        inline uint32_t getRemaining() const;
        inline uint32_t getMaxSetNb() const;

      private:
        std::vector<VkDescriptorSet> m_descriptors;
        uint32_t                     m_maxSetNb = 0;
        View<DescriptorLayout>       m_layout   = {};
    };

} // namespace vzt

#include "vzt/Vulkan/Descriptor.inl"

#endif // VZT_VULKAN_DESCRIPTOR_HPP
