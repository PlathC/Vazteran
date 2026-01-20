#ifndef VZT_VULKAN_PIPELINE_PIPEINE_HPP
#define VZT_VULKAN_PIPELINE_PIPEINE_HPP

#include "vzt/vulkan/descriptor.hpp"
#include "vzt/vulkan/device.hpp"

namespace vzt
{
    class pipeline : public DeviceObject<VkPipeline>
    {
      public:
        pipeline() = default;
        inline pipeline(View<Device> device);

        inline pipeline(pipeline&& other) noexcept;
        inline pipeline& operator=(pipeline&& other) noexcept;

        virtual ~pipeline() = default;
        inline const DescriptorLayout& getDescriptorLayout() const;
        inline VkPipelineLayout        getLayout() const;

      protected:
        DescriptorLayout m_descriptorLayout;
        VkPipelineLayout m_pipelineLayout;
    };
} // namespace vzt

#include "vzt/vulkan/pipeline/pipeline.inl"

#endif // VZT_VULKAN_PIPELINE_PIPEINE_HPP