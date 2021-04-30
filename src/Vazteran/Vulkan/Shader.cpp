#include "Vazteran/Vulkan/DeviceManager.hpp"
#include "Vazteran/Vulkan/Shader.hpp"

namespace vzt {
    Shader::Shader(DeviceManager* deviceManager, const fs::path& compiled_file, ShaderStage shaderStage):
            m_deviceManager(deviceManager), m_shaderStage(shaderStage) {
        m_compiledSource = vzt::ReadFile(compiled_file);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = m_compiledSource.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(m_compiledSource.data());

        if (vkCreateShaderModule(m_deviceManager->LogicalDevice(), &createInfo, nullptr, &m_shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }

        m_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        m_stage.stage = static_cast<VkShaderStageFlagBits>(m_shaderStage);
        m_stage.module = m_shaderModule;
        m_stage.pName = "main";
    }

    Shader::~Shader() {
        vkDestroyShaderModule(m_deviceManager->LogicalDevice(), m_shaderModule, nullptr);
    }
}