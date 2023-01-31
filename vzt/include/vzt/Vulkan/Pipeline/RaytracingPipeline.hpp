#ifndef VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
#define VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Descriptor.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    enum class ShaderGroupType : uint8_t
    {
        General            = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
        TrianglesHitGroup  = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
        ProceduralHitGroup = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ShaderGroupType, VkRayTracingShaderGroupTypeKHR);

    class RaytracingPipeline
    {
      public:
        RaytracingPipeline(View<Device> device);

        RaytracingPipeline(const RaytracingPipeline&)            = delete;
        RaytracingPipeline& operator=(const RaytracingPipeline&) = delete;

        RaytracingPipeline(RaytracingPipeline&& other) noexcept;
        RaytracingPipeline& operator=(RaytracingPipeline&& other) noexcept;

        ~RaytracingPipeline();

        void        addShader(Shader shader, ShaderGroupType hitGroupType = ShaderGroupType::General);
        inline void setDescriptorLayout(DescriptorLayout descriptorLayout);

        void compile();

      private:
        void cleanup();

        View<Device>     m_device;
        VkPipeline       m_handle         = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        DescriptorLayout m_descriptorLayout;

        std::vector<ShaderModule>                         m_shaderModules;
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups;

        bool m_compiled = false;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline/RaytracingPipeline.inl"

#endif // VZT_VULKAN_PIPELINE_RAYTRACINGPIPELINE_HPP
