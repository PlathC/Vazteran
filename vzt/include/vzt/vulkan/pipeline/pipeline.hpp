#ifndef VZT_VULKAN_PIPELINE_PIPEINE_HPP
#define VZT_VULKAN_PIPELINE_PIPEINE_HPP

#include "vzt/vulkan/descriptor.hpp"
#include "vzt/vulkan/device.hpp"

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
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    };
} // namespace vzt

#include "vzt/vulkan/pipeline/pipeline.inl"

#endif // VZT_VULKAN_PIPELINE_PIPEINE_HPP