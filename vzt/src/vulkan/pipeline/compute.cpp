#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/Program.hpp"
#include "vzt/Vulkan/pipeline/compute.hpp"

namespace vzt
{
    compute::compute(const Program& program) : pipeline(program.getModules()[0].getDevice())
    {
        setProgram(program);
    }

    compute::compute(compute&& other) noexcept
        : pipeline(std::move(other)), m_program(std::move(other.m_program)), m_compiled(std::move(other.m_compiled))
    {
    }

    compute& compute::operator=(compute&& other) noexcept
    {
        std::swap(m_program, other.m_program);
        std::swap(m_compiled, other.m_compiled);

        pipeline::operator=(std::move(other));
        return *this;
    }

    compute::~compute() { cleanup(); }

    void compute::setProgram(const Program& program)
    {
        m_program = program;

        m_descriptorLayout = vzt::DescriptorLayout(m_device);
        for (const auto& module : program.getModules())
        {
            const auto& shader = module.getShader();
            for (const auto [id, type] : shader.bindings)
                m_descriptorLayout.addBinding(id, type);
        }

        m_descriptorLayout.compile();
    }

    void compute::compile()
    {
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

    void compute::cleanup()
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
