#ifndef VZT_COMPUTE_PIPELINE_HPP
#define VZT_COMPUTE_PIPELINE_HPP

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Pipeline/Pipeline.hpp"

namespace vzt
{
    class Device;
    class Program;

    class ComputePipeline : public Pipeline
    {
      public:
        ComputePipeline() = default;
        ComputePipeline(View<Device> device);

        ComputePipeline(const ComputePipeline&)            = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        ComputePipeline(ComputePipeline&&) noexcept;
        ComputePipeline& operator=(ComputePipeline&&) noexcept;

        ~ComputePipeline();

        void setProgram(const Program& program);
        void compile();

        inline VkPipeline       getHandle() const;
        inline VkPipelineLayout getLayout() const;

      private:
        void cleanup();

        View<Device>     m_device;
        VkPipeline       m_handle         = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        View<Program>    m_program;

        bool m_compiled = false;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline/ComputePipeline.inl"

#endif // VZT_COMPUTE_PIPELINE_HPP
