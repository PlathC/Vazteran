#ifndef VZT_COMPUTE_PIPELINE_HPP
#define VZT_COMPUTE_PIPELINE_HPP

#include "vzt/core/type.hpp"
#include "vzt/vulkan/pipeline/pipeline.hpp"

namespace vzt
{
    class Device;
    class Program;

    struct ComputePipelineBuilder : PipelineBuilder<ComputePipelineBuilder>
    {
        ComputePipelineBuilder(View<Program> program = {}) : program(program) {}

        View<Program> program;
    };

    class ComputePipeline : public Pipeline
    {
      public:
        ComputePipeline() = default;
        ComputePipeline(ComputePipelineBuilder builder);

        ComputePipeline(const ComputePipeline&)            = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        ComputePipeline(ComputePipeline&&) noexcept;
        ComputePipeline& operator=(ComputePipeline&&) noexcept;

      private:
        void compile();

        View<Program> m_program;
        bool          m_compiled = false;
    };
} // namespace vzt

#endif // VZT_COMPUTE_PIPELINE_HPP
