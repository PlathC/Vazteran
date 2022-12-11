#include "vzt/Vulkan/Pipeline/ComputePipeline.hpp"

#include "vzt/Vulkan/Device.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    ComputePipeline::ComputePipeline(View<Device> device, View<Program> program) : m_device(device), m_program(program)
    {
    }

    ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept
        : m_device(std::move(other.m_device)), m_handle(std::move(other.m_handle)),
          m_pipelineLayout(std::move(other.m_pipelineLayout)), m_program(std::move(other.m_program)),
          m_descriptorLayout(std::move(other.m_descriptorLayout)), m_compiled(std::move(other.m_compiled))
    {
    }

    ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_handle, other.m_handle);
        std::swap(m_pipelineLayout, other.m_pipelineLayout);
        std::swap(m_program, other.m_program);
        std::swap(m_descriptorLayout, other.m_descriptorLayout);
        std::swap(m_compiled, other.m_compiled);

        return *this;
    }

    ComputePipeline::~ComputePipeline() { cleanup(); }

    void ComputePipeline::compile()
    {
        VkDescriptorSetLayout descriptorSetLayout = m_descriptorLayout.getHandle();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;

        vkCheck(vkCreatePipelineLayout(m_device->getHandle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
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

        vkCheck(vkCreateComputePipelines(m_device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_handle),
                "Failed to create compute pipeline.");

        m_compiled = true;
    }

    void ComputePipeline::cleanup()
    {
        if (m_handle != VK_NULL_HANDLE)
            vkDestroyPipeline(m_device->getHandle(), m_handle, nullptr);

        if (m_pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(m_device->getHandle(), m_pipelineLayout, nullptr);

        m_compiled = false;
    }

} // namespace vzt
