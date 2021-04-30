#ifndef VAZTERAN_SHADER_HPP
#define VAZTERAN_SHADER_HPP

#include "Vazteran/Utils.hpp"

#include <vulkan/vulkan.h>

namespace vzt {
    class DeviceManager;

    enum ShaderStage {
        VertexShader = VK_SHADER_STAGE_VERTEX_BIT,
        FragmentShader = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    class Shader {
    public:
        Shader(DeviceManager* deviceManager, const fs::path& compiled_file, ShaderStage shaderStage);
        VkPipelineShaderStageCreateInfo Stage() const { return m_stage; }
        ~Shader();
    private:
        DeviceManager* m_deviceManager;
        std::vector<char> m_compiledSource;
        VkShaderModule m_shaderModule;
        VkPipelineShaderStageCreateInfo m_stage{};
        ShaderStage m_shaderStage;
    };
}

#endif //VAZTERAN_SHADER_HPP
