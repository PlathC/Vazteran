#ifndef VAZTERAN_SHADER_HPP
#define VAZTERAN_SHADER_HPP

#include "Vazteran/Utils.hpp"

#include <vulkan/vulkan.h>

namespace vzt {
    class LogicalDevice;

    enum ShaderStage {
        VertexShader = VK_SHADER_STAGE_VERTEX_BIT,
        FragmentShader = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    class Shader {
    public:
        Shader(LogicalDevice* logicalDevice, const fs::path& compiled_file, ShaderStage shaderStage);

        VkPipelineShaderStageCreateInfo Stage() const { return m_stage; }

        ~Shader();
    private:
        LogicalDevice* m_logicalDevice;
        std::vector<char> m_compiledSource;
        VkShaderModule m_shaderModule;
        VkPipelineShaderStageCreateInfo m_stage{};
        ShaderStage m_shaderStage;
    };
}

#endif //VAZTERAN_SHADER_HPP
