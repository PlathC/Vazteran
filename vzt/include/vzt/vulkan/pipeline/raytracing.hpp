#ifndef VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
#define VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP

#include "vzt/core/type.hpp"
#include "vzt/vulkan/pipeline/pipeline.hpp"
#include "vzt/vulkan/program.hpp"

namespace vzt
{
    struct RaytracingPipelineBuilder : PipelineBuilder<RaytracingPipelineBuilder>
    {
        RaytracingPipelineBuilder(View<ShaderGroup> shaderGroup = {}) : shaderGroup(shaderGroup) {}

        View<ShaderGroup> shaderGroup;
        uint32_t          maxRecursionDepth = 1;
    };

    class RaytracingPipeline : public Pipeline
    {
      public:
        RaytracingPipeline(RaytracingPipelineBuilder builder);

        RaytracingPipeline(const RaytracingPipeline&)            = delete;
        RaytracingPipeline& operator=(const RaytracingPipeline&) = delete;

        RaytracingPipeline(RaytracingPipeline&& other) noexcept;
        RaytracingPipeline& operator=(RaytracingPipeline&& other) noexcept;

        inline CSpan<uint8_t> getShaderHandleStorage() const;
        inline uint32_t       getShaderHandleSize() const;
        inline uint32_t       getShaderHandleSizeAligned() const;

      private:
        void compile();

        View<ShaderGroup> m_shaderGroup;
        uint32_t          m_maxRecursionDepth = 1;

        uint32_t             m_handleSize          = {};
        uint32_t             m_handleSizeAligned   = {};
        std::vector<uint8_t> m_shaderHandleStorage = {};
    };
} // namespace vzt

#include "vzt/vulkan/pipeline/raytracing.inl"

#endif // VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
