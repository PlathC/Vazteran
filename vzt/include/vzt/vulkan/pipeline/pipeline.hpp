#ifndef VZT_VULKAN_PIPELINE_PIPEINE_HPP
#define VZT_VULKAN_PIPELINE_PIPEINE_HPP

#include "vzt/vulkan/descriptor.hpp"
#include "vzt/vulkan/device.hpp"

namespace vzt
{
    struct PushConstant
    {
        template <class Type>
        static PushConstant Typed(ShaderStage stages);

        ShaderStage stages;
        uint32_t    offset;
        uint32_t    size;
    };

    class Pipeline : public DeviceObject<VkPipeline>
    {
      public:
        Pipeline() = default;
        inline Pipeline(View<Device> device);

        inline Pipeline(Pipeline&& other) noexcept;
        inline Pipeline& operator=(Pipeline&& other) noexcept;

        virtual ~Pipeline() = default;
        inline void                    add(PushConstant constant);
        inline const DescriptorLayout& getDescriptorLayout() const;
        inline VkPipelineLayout        getLayout() const;

      protected:
        DescriptorLayout m_descriptorLayout;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        std::vector<PushConstant> m_pushConstants;
    };
} // namespace vzt

#include "vzt/vulkan/pipeline/pipeline.inl"

#endif // VZT_VULKAN_PIPELINE_PIPEINE_HPP