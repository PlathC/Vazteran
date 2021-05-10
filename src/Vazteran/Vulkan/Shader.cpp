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

    ShaderModule::ShaderModule(ShaderModule&& other) noexcept {
        m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
        std::swap(m_logicalDevice, other.m_logicalDevice);
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
        std::swap(m_vkHandle, other.m_vkHandle);
        std::swap(m_logicalDevice, other.m_logicalDevice);

        return *this;
    }

    ShaderModule::~ShaderModule() {
        if (m_vkHandle != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
        }
    }

}
