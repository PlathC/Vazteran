#ifndef VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
#define VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Pipeline/Pipeline.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    class RaytracingPipeline : public Pipeline
    {
      public:
        RaytracingPipeline(View<Device> device);

        RaytracingPipeline(const RaytracingPipeline&)            = delete;
        RaytracingPipeline& operator=(const RaytracingPipeline&) = delete;

        RaytracingPipeline(RaytracingPipeline&& other) noexcept;
        RaytracingPipeline& operator=(RaytracingPipeline&& other) noexcept;

        ~RaytracingPipeline();

        inline void setShaderGroup(const ShaderGroup& shaderGroup);

        void                    compile();
        inline VkPipeline       getHandle() const;
        inline VkPipelineLayout getLayout() const;
        inline CSpan<uint8_t>   getShaderHandleStorage() const;
        inline uint32_t         getShaderHandleSize() const;
        inline uint32_t         getShaderHandleSizeAligned() const;

      private:
        void cleanup();

        View<Device>     m_device;
        VkPipeline       m_handle         = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        View<ShaderGroup> m_shaderGroup;

        uint32_t             m_handleSize;
        uint32_t             m_handleSizeAligned;
        std::vector<uint8_t> m_shaderHandleStorage;

        bool m_compiled = false;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline/RaytracingPipeline.inl"

#endif // VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
