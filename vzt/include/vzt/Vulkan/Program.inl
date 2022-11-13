#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    inline std::size_t Shader::hash::operator()(const Shader& handle) const
    {
        return static_cast<uint8_t>(handle.stage);
    }

    inline VkShaderModule ShaderModule::getHandle() const { return m_handle; }
} // namespace vzt
