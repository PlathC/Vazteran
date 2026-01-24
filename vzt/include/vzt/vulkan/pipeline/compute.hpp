#ifndef VZT_COMPUTE_PIPELINE_HPP
#define VZT_COMPUTE_PIPELINE_HPP

#include "vzt/core/type.hpp"
#include "vzt/vulkan/pipeline/pipeline.hpp"

namespace vzt
{
    class Device;
    class Program;

    class ComputePipeline : public Pipeline
    {
      public:
        ComputePipeline() = default;
        ComputePipeline(const Program& program);

        ComputePipeline(const ComputePipeline&)            = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        ComputePipeline(ComputePipeline&&) noexcept;
        ComputePipeline& operator=(ComputePipeline&&) noexcept;

        ~ComputePipeline() override;

      private:
        void compile();

        View<Program> m_program;
        bool          m_compiled = false;
    };
} // namespace vzt

#endif // VZT_COMPUTE_PIPELINE_HPP
