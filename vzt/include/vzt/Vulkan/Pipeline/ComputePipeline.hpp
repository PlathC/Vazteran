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
        ComputePipeline(const Program& program);

        ComputePipeline(const ComputePipeline&)            = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        ComputePipeline(ComputePipeline&&) noexcept;
        ComputePipeline& operator=(ComputePipeline&&) noexcept;

        ~ComputePipeline() override;

        void setProgram(const Program& program);
        void compile();

      private:
        void cleanup();

        View<Program> m_program;
        bool          m_compiled = false;
    };
} // namespace vzt

#endif // VZT_COMPUTE_PIPELINE_HPP
