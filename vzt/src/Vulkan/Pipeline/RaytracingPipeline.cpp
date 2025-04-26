#include "vzt/Vulkan/Pipeline/RaytracingPipeline.hpp"

#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    RaytracingPipeline::RaytracingPipeline(View<Device> device) : Pipeline(), m_device(device) {}

    RaytracingPipeline::RaytracingPipeline(RaytracingPipeline&& other) noexcept : Pipeline(std::move(other))
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_compiled, other.m_compiled);
    }

    RaytracingPipeline& RaytracingPipeline::operator=(RaytracingPipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_compiled, other.m_compiled);

        Pipeline::operator=(std::move(other));

        return *this;
    }

    RaytracingPipeline::~RaytracingPipeline() { cleanup(); }

    void RaytracingPipeline::setShaderGroup(const ShaderGroup& shaderGroup)
    {
        m_shaderGroup = shaderGroup;

        m_descriptorLayout = vzt::DescriptorLayout(m_device);
        for (const auto& group : shaderGroup.getShaders())
        {
            const Shader& shader = group.shaderModule.getShader();
            for (const auto [id, type] : shader.bindings)
                m_descriptorLayout.addBinding(id, type);
        }

        m_descriptorLayout.compile();
    }

    void RaytracingPipeline::compile()
    {
        cleanup();

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;

        const VkDescriptorSetLayout layout = m_descriptorLayout.getHandle();
        pipelineLayoutCI.pSetLayouts       = &layout;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreatePipelineLayout(m_device->getHandle(), &pipelineLayoutCI, nullptr, &m_pipelineLayout),
                "Can't create pipeline layout");

        CSpan<ShaderGroupShader> shaders = m_shaderGroup->getShaders();

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        shaderStages.reserve(shaders.size);
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
        shaderGroups.reserve(shaders.size);
        for (const ShaderGroupShader& shaderGroupShader : shaders)
        {
            const ShaderModule& shaderModule = shaderGroupShader.shaderModule;
            const ShaderStage   stage        = shaderModule.getShader().stage;
            const uint32_t      index        = static_cast<uint32_t>(shaderStages.size());

            VkRayTracingShaderGroupCreateInfoKHR shaderGroup;
            shaderGroup.sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroup.pNext              = nullptr;
            shaderGroup.type               = toVulkan(shaderGroupShader.hitGroupType);
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
                assert(shaderGroup.type == toVulkan(ShaderGroupType::ProceduralHitGroup) ||
                       shaderGroup.type == toVulkan(ShaderGroupType::TrianglesHitGroup) &&
                           "Vulkan specification state that if an ClosestHit shader is provided, "
                           "hitGroupType must be equal to ShaderGroupType::ProceduralHitGroup or "
                           "ShaderGroupType::TrianglesHitGroup");

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

            default: throw std::runtime_error("Unkown shader stage");
            }

            shaderGroups.emplace_back(std::move(shaderGroup));

            VkPipelineShaderStageCreateInfo createInfo{};
            createInfo.module = shaderGroupShader.shaderModule.getHandle();
            createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage  = toVulkan(stage);
            createInfo.pName  = "main";
            shaderStages.emplace_back(createInfo);
        }

        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
        rayTracingPipelineCI.sType                        = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rayTracingPipelineCI.stageCount                   = static_cast<uint32_t>(shaderStages.size());
        rayTracingPipelineCI.pStages                      = shaderStages.data();
        rayTracingPipelineCI.groupCount                   = static_cast<uint32_t>(shaderGroups.size());
        rayTracingPipelineCI.pGroups                      = shaderGroups.data();
        rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
        rayTracingPipelineCI.layout                       = m_pipelineLayout;

        vkCheck(
            table.vkCreateRayTracingPipelinesKHR( //
                m_device->getHandle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &m_handle),
            "Can't create raytracing pipeline.");

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
        rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &rayTracingPipelineProperties;
        vkGetPhysicalDeviceProperties2(m_device->getHardware().getHandle(), &deviceProperties2);

        m_handleSize              = rayTracingPipelineProperties.shaderGroupHandleSize;
        m_handleSizeAligned       = static_cast<uint32_t>(align( //
            rayTracingPipelineProperties.shaderGroupHandleSize,
            rayTracingPipelineProperties.shaderGroupHandleAlignment));
        const uint32_t groupCount = static_cast<uint32_t>(m_shaderGroup->size());
        const uint32_t sbtSize    = groupCount * m_handleSizeAligned;

        m_shaderHandleStorage = std::vector<uint8_t>(sbtSize);
        vkCheck(vkGetRayTracingShaderGroupHandlesKHR(m_device->getHandle(), m_handle, 0, groupCount, sbtSize,
                                                     m_shaderHandleStorage.data()),
                "Can't get shader group handles.");

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
