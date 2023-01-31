#include "vzt/Vulkan/Pipeline/RaytracingPipeline.hpp"

#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    RaytracingPipeline::RaytracingPipeline(View<Device> device) : m_device(device) {}

    RaytracingPipeline::RaytracingPipeline(RaytracingPipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_shaderModules, other.m_shaderModules);
        std::swap(m_shaderGroups, other.m_shaderGroups);
        std::swap(m_compiled, other.m_compiled);
    }

    RaytracingPipeline& RaytracingPipeline::operator=(RaytracingPipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_shaderModules, other.m_shaderModules);
        std::swap(m_shaderGroups, other.m_shaderGroups);
        std::swap(m_compiled, other.m_compiled);

        return *this;
    }

    RaytracingPipeline::~RaytracingPipeline() { cleanup(); }

    void RaytracingPipeline::addShader(Shader shader, ShaderGroupType hitGroupType)
    {
        const ShaderStage stage = shader.stage;
        const uint32_t    index = static_cast<uint32_t>(m_shaderGroups.size());

        VkRayTracingShaderGroupCreateInfoKHR shaderGroup;
        shaderGroup.type               = toVulkan(hitGroupType);
        shaderGroup.generalShader      = VK_SHADER_UNUSED_KHR;
        shaderGroup.closestHitShader   = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader       = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        switch (stage)
        {
        // generalShader is the index of the ray generation, miss, or callable shader
        case ShaderStage::RayGen:
        case ShaderStage::Miss:
        case ShaderStage::Callable: //
            shaderGroup.generalShader = index;
            break;

            // closestHitShader is the optional index of the closest hit shader
        case ShaderStage::ClosestHit: //
            shaderGroup.closestHitShader = index;
            break;

        case ShaderStage::AnyHit: //
            assert(shaderGroup.type == toVulkan(ShaderGroupType::ProceduralHitGroup) ||
                   shaderGroup.type == toVulkan(ShaderGroupType::TrianglesHitGroup) &&
                       "Vulkan specification state that if an AnyHit shader is provided, "
                       "hitGroupType must be equal to ShaderGroupType::ProceduralHitGroup or "
                       "ShaderGroupType::TrianglesHitGroup");

            shaderGroup.anyHitShader = index;
            break;

        // intersectionShader is the index of the intersection shader
        case ShaderStage::Intersection: //
            assert(shaderGroup.type == toVulkan(ShaderGroupType::ProceduralHitGroup) &&
                   "Vulkan specification state that if an intersection shader is provided, "
                   "hitGroupType must be equal to ShaderGroupType::ProceduralHitGroup");
            shaderGroup.intersectionShader = index;
            break;
        }

        m_shaderGroups.emplace_back(std::move(shaderGroup));
        m_shaderModules.emplace_back(ShaderModule(m_device, std::move(shader)));
    }

    void RaytracingPipeline::compile()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;

        const VkDescriptorSetLayout layout = m_descriptorLayout.getHandle();
        pipelineLayoutCI.pSetLayouts       = &layout;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreatePipelineLayout(m_device->getHandle(), &pipelineLayoutCI, nullptr, &m_pipelineLayout),
                "Can't create pipeline layout");

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        shaderStages.reserve(m_shaderModules.size());
        for (const auto& shaderModule : m_shaderModules)
        {
            const auto& shader = shaderModule.getShader();

            VkPipelineShaderStageCreateInfo createInfo{};
            createInfo.module = shaderModule.getHandle();
            createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage  = toVulkan(shader.stage);
            createInfo.pName  = "main";
            shaderStages.emplace_back(createInfo);
        }

        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
        rayTracingPipelineCI.sType                        = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rayTracingPipelineCI.stageCount                   = static_cast<uint32_t>(shaderStages.size());
        rayTracingPipelineCI.pStages                      = shaderStages.data();
        rayTracingPipelineCI.groupCount                   = static_cast<uint32_t>(m_shaderGroups.size());
        rayTracingPipelineCI.pGroups                      = m_shaderGroups.data();
        rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
        rayTracingPipelineCI.layout                       = m_pipelineLayout;

        vkCheck(
            table.vkCreateRayTracingPipelinesKHR( //
                m_device->getHandle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &m_handle),
            "Can't create raytracing pipeline.");

        m_compiled = true;
    }

    void RaytracingPipeline::cleanup()
    {
        if (m_handle != VK_NULL_HANDLE)
        {
            const VolkDeviceTable& table = m_device->getFunctionTable();
            table.vkDestroyPipeline(m_device->getHandle(), m_handle, nullptr);
        }

        if (m_pipelineLayout != VK_NULL_HANDLE)
        {
            const VolkDeviceTable& table = m_device->getFunctionTable();
            table.vkDestroyPipelineLayout(m_device->getHandle(), m_pipelineLayout, nullptr);
        }

        m_compiled = false;
    }
} // namespace vzt
