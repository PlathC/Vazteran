#include "vzt/vulkan/program.hpp"

namespace vzt
{
    inline std::size_t Shader::hash::operator()(const Shader& handle) const
    {
        return static_cast<uint8_t>(handle.stage);
    }

    inline VkShaderModule ShaderModule::getHandle() const { return m_handle; }
    inline const Shader&  ShaderModule::getShader() const { return m_shader; }

    inline void Program::setShader(Shader shader)
    {
        m_shaderModules.emplace_back(ShaderModule(m_device, std::move(shader)));
    }
    inline const std::vector<ShaderModule>& Program::getModules() const { return m_shaderModules; }

    inline CSpan<ShaderGroupShader> ShaderGroup::getShaders() const { return m_shaders; }
    inline std::size_t              ShaderGroup::size() const { return m_shaders.size(); }
} // namespace vzt
