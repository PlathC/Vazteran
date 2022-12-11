#ifndef VZT_COMPUTE_PIPELINE_HPP
#define VZT_COMPUTE_PIPELINE_HPP

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Descriptor.hpp"

namespace vzt
{
    class Device;
    class Program;

    class ComputePipeline
    {
      public:
        ComputePipeline() = default;
        ComputePipeline(View<Device> device, View<Program> program);

        ComputePipeline(const ComputePipeline&)            = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        ComputePipeline(ComputePipeline&&) noexcept;
        ComputePipeline& operator=(ComputePipeline&&) noexcept;

        ~ComputePipeline();

        inline void setProgram(const Program& program);
        inline void setDescriptorLayout(DescriptorLayout descriptorLayout);

        void compile();

        inline VkPipeline       getHandle() const;
        inline VkPipelineLayout getLayout() const;

      private:
        void cleanup();

        View<Device>     m_device;
        VkPipeline       m_handle         = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        View<Program>    m_program;
        DescriptorLayout m_descriptorLayout;

        bool m_compiled = false;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline/ComputePipeline.inl"

#endif // VZT_COMPUTE_PIPELINE_HPP
