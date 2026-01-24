#ifndef VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
#define VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP

#include "vzt/core/type.hpp"
#include "vzt/vulkan/pipeline/pipeline.hpp"
#include "vzt/vulkan/program.hpp"

namespace vzt
{
    class RaytracingPipeline : public Pipeline
    {
      public:
        RaytracingPipeline(const ShaderGroup& shaderGroup);

        RaytracingPipeline(const RaytracingPipeline&)            = delete;
        RaytracingPipeline& operator=(const RaytracingPipeline&) = delete;

        RaytracingPipeline(RaytracingPipeline&& other) noexcept;
        RaytracingPipeline& operator=(RaytracingPipeline&& other) noexcept;

        ~RaytracingPipeline() override;

        inline void setShaderGroup(const ShaderGroup& shaderGroup);

        void                  compile();
        inline CSpan<uint8_t> getShaderHandleStorage() const;
        inline uint32_t       getShaderHandleSize() const;
        inline uint32_t       getShaderHandleSizeAligned() const;

      private:
        void cleanup();

        View<ShaderGroup> m_shaderGroup;

        uint32_t             m_handleSize          = {};
        uint32_t             m_handleSizeAligned   = {};
        std::vector<uint8_t> m_shaderHandleStorage = {};

        bool m_compiled = false;
    };
} // namespace vzt

#include "vzt/vulkan/pipeline/raytracing.inl"

#endif // VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
