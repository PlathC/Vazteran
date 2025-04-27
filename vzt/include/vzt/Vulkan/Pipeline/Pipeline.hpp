#ifndef VZT_VULKAN_PIPELINE_PIPEINE_HPP
#define VZT_VULKAN_PIPELINE_PIPEINE_HPP

#include "vzt/Vulkan/Descriptor.hpp"
#include "vzt/Vulkan/DeviceObject.hpp"

namespace vzt
{
    class Pipeline : public DeviceObject<VkPipeline>
    {
      public:
        Pipeline() = default;
        inline Pipeline(View<Device> device);

        inline Pipeline(Pipeline&& other) noexcept;
        inline Pipeline& operator=(Pipeline&& other) noexcept;

        virtual ~Pipeline() = default;
        inline const DescriptorLayout& getDescriptorLayout() const;
        inline VkPipelineLayout        getLayout() const;

      protected:
        DescriptorLayout m_descriptorLayout;
        VkPipelineLayout m_pipelineLayout;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline/Pipeline.inl"

#endif // VZT_VULKAN_PIPELINE_PIPEINE_HPP