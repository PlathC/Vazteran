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

    template <class Derived>
    struct PipelineBuilder
    {
        std::vector<PushConstant> pushConstants = {};

        Derived& add(PushConstant constant)
        {
            pushConstants.emplace_back(constant);
            return static_cast<Derived&>(*this);
        }
    };

    class Pipeline : public DeviceObject<VkPipeline>
    {
      public:
        Pipeline() = default;
        inline Pipeline(View<Device> device);

        inline Pipeline(Pipeline&& other) noexcept;
        inline Pipeline& operator=(Pipeline&& other) noexcept;

        ~Pipeline() override;
        inline void                    add(PushConstant constant);
        inline const DescriptorLayout& getDescriptorLayout() const;
        inline VkPipelineLayout        getLayout() const;

      protected:
        void createLayout();
        // Destroys the pipeline and its layout
        void destroy();

        DescriptorLayout m_descriptorLayout;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        std::vector<PushConstant> m_pushConstants;
    };
} // namespace vzt

#include "vzt/vulkan/pipeline/pipeline.inl"

#endif // VZT_VULKAN_PIPELINE_PIPEINE_HPP