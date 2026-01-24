#include "vzt/vulkan/pipeline/compute.hpp"

#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/program.hpp"

namespace vzt
{
    ;
    ComputePipeline::ComputePipeline(const Program& program)
        : Pipeline(program.getModules()[0].getDevice()), m_program(program)
    {
        compile();
    }

    ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept
        : Pipeline(std::move(other)), m_program(std::move(other.m_program)), m_compiled(std::move(other.m_compiled))
    {
    }

    ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) noexcept
    {
        std::swap(m_program, other.m_program);
        std::swap(m_compiled, other.m_compiled);

        Pipeline::operator=(std::move(other));
        return *this;
    }

    ComputePipeline::~ComputePipeline()
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
    }

    void ComputePipeline::compile()
    {
        m_descriptorLayout = vzt::DescriptorLayout(m_device);
        for (const auto& module : m_program->getModules())
        {
            const auto& shader = module.getShader();
            for (const auto [id, type] : shader.bindings)
                m_descriptorLayout.addBinding(id, type);
        }

        m_descriptorLayout.compile();

        VkDescriptorSetLayout descriptorSetLayout = m_descriptorLayout.getHandle();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        vkCheck(table.vkCreatePipelineLayout(m_device->getHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
                "failed to create pipeline layout!");

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.flags  = 0;

        VkPipelineShaderStageCreateInfo createInfo{};
        const auto&                     shaderModules = m_program->getModules();

        assert(shaderModules.size() == 1 && "Compute pipeline must contains 1 shaders.");

        const auto& shaderModule = shaderModules.back();
        const auto& shader       = shaderModule.getShader();
        createInfo.module        = shaderModule.getHandle();
        createInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage         = toVulkan(shader.stage);
        createInfo.pName         = "main";

        pipelineInfo.stage = createInfo;

        vkCheck(
            table.vkCreateComputePipelines(m_device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_handle),
            "Failed to create compute pipeline.");

        m_compiled = true;
    }

} // namespace vzt
