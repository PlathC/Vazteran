#include <utility>

#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/Shader.hpp"

namespace vzt {
    Shader::Shader(const fs::path& compiled_file, vzt::ShaderStage shaderStage):
            m_shaderStage(shaderStage) {
        m_compiledSource = vzt::ReadFile(compiled_file);

        m_shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        m_shaderModuleCreateInfo.codeSize = m_compiledSource.size();
        m_shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_compiledSource.data());
    }

    void Shader::SetSamplerDescriptorSet(uint32_t binding, const vzt::Image& image) {
        for (auto& sampler: m_samplerDescriptorSets) {
            if (sampler.binding == binding){
                return;
            }
        }

        // If this binding is not currently used
        m_samplerDescriptorSets.emplace_back(SamplerDescriptorSet{binding});
    }

    void Shader::SetUniformDescriptorSet(uint32_t binding, uint32_t size) {
        for (auto& uniform: m_uniformDescriptorSets) {
            if (uniform.binding == binding)
                uniform.size = size;
            return;
        }

        // If this binding is not currently used
        m_uniformDescriptorSets.emplace_back(UniformDescriptorSet{binding, size});
    }

    std::vector<std::pair<uint32_t, VkDescriptorType>> Shader::DescriptorTypes() const {
        std::vector<std::pair<uint32_t, VkDescriptorType>> descriptorTypes;
        for (const auto& samplerDescriptorSet: m_samplerDescriptorSets)
            descriptorTypes.emplace_back(samplerDescriptorSet.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

        for (const auto& samplerDescriptorSet: m_uniformDescriptorSets)
            descriptorTypes.emplace_back(samplerDescriptorSet.binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        return descriptorTypes;
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
