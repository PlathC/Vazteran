#ifndef VZT_COMPUTE_PIPELINE_HPP
#define VZT_COMPUTE_PIPELINE_HPP

#include "vzt/core/type.hpp"
#include "vzt/vulkan/pipeline/pipeline.hpp"

namespace vzt
{
    class Device;
    class Program;

    class compute : public pipeline
    {
      public:
        compute() = default;
        compute(const Program& program);

        compute(const compute&)            = delete;
        compute& operator=(const compute&) = delete;

        compute(compute&&) noexcept;
        compute& operator=(compute&&) noexcept;

        ~compute() override;

        void setProgram(const Program& program);
        void compile();

      private:
        void cleanup();

        View<Program> m_program;
        bool          m_compiled = false;
    };
} // namespace vzt

#endif // VZT_COMPUTE_PIPELINE_HPP
