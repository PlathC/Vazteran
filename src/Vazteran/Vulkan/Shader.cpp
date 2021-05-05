#include <utility>

#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/Shader.hpp"

namespace vzt {
    Shader::Shader(const fs::path& compiled_file, vzt::ShaderStage shaderStage,
                   std::vector<SamplerDescriptorSet> samplerDescriptorSets,
                   std::vector<UniformDescriptorSet> uniformDescriptorSets):
            m_shaderStage(shaderStage), m_samplerDescriptorSets(std::move(samplerDescriptorSets)),
            m_uniformDescriptorSets(std::move(uniformDescriptorSets)) {
        m_compiledSource = vzt::ReadFile(compiled_file);

        m_shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        m_shaderModuleCreateInfo.codeSize = m_compiledSource.size();
        m_shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_compiledSource.data());
    }

    ShaderModule::ShaderModule(vzt::LogicalDevice* logicalDevice, VkShaderModuleCreateInfo createInfo) :
            m_logicalDevice(logicalDevice) {
        if (vkCreateShaderModule(m_logicalDevice->VkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }
    }

    ShaderModule::~ShaderModule() {
        vkDestroyShaderModule(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
    }

}
