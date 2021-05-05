#ifndef VAZTERAN_SHADER_HPP
#define VAZTERAN_SHADER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Utils.hpp"
#include "Vazteran/Vulkan/Texture.hpp"

namespace vzt {
    class LogicalDevice;

    enum ShaderStage {
        VertexShader = VK_SHADER_STAGE_VERTEX_BIT,
        TesselationControlShader = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        TesselationEvaluationShader = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        GeometryShader = VK_SHADER_STAGE_GEOMETRY_BIT,
        FragmentShader = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    struct DescriptorSet {
        uint32_t binding;
    };

    struct SamplerDescriptorSet : public DescriptorSet {
        vzt::Image image;
    };

    struct UniformDescriptorSet : public DescriptorSet {
        uint32_t size;
    };

    class Shader {
    public:
        Shader(const fs::path& compiled_file,
               vzt::ShaderStage shaderStage, std::vector<SamplerDescriptorSet> samplerDescriptorSets,
               std::vector<UniformDescriptorSet> uniformDescriptorSets);

        VkShaderModuleCreateInfo ShaderModuleCreateInfo() const { return m_shaderModuleCreateInfo; }
        vzt::ShaderStage Stage() const { return m_shaderStage; }
        std::vector<SamplerDescriptorSet> SamplerDescriptorSets() const { return m_samplerDescriptorSets; }
        std::vector<UniformDescriptorSet> UniformDescriptorSets() const { return m_uniformDescriptorSets; }

        bool operator==(const Shader& other) const
        {
            return m_shaderStage == other.Stage();
        }

    private:
        std::vector<char> m_compiledSource;
        VkShaderModule m_shaderModule{};
        VkShaderModuleCreateInfo m_shaderModuleCreateInfo{};
        vzt::ShaderStage m_shaderStage;
        std::vector<SamplerDescriptorSet> m_samplerDescriptorSets;
        std::vector<UniformDescriptorSet> m_uniformDescriptorSets;
    };

    struct ShaderHash
    {
        std::size_t operator()(const Shader& shader) const
        {
            return static_cast<std::size_t>(shader.Stage());
        }
    };

    class ShaderModule {
    public:
        ShaderModule(vzt::LogicalDevice* logicalDevice, VkShaderModuleCreateInfo createInfo);

        VkShaderModule VkHandle() const { return m_vkHandle; }

        ~ShaderModule();

    private:
        VkShaderModule m_vkHandle;
        vzt::LogicalDevice* m_logicalDevice;
    };
}

#endif //VAZTERAN_SHADER_HPP
