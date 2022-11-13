#include "vzt/Vulkan/Program.hpp"

#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    ShaderModule::ShaderModule(View<Device> device, Shader shader) : m_device(device)
    {
        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = shader.compiledSource.size() * sizeof(uint32_t);
        shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(shader.compiledSource.data());

        vkCheck(vkCreateShaderModule(m_device->getHandle(), &shaderModuleCreateInfo, nullptr, &m_handle),
                "Failed to create shader module.");
    }

    ShaderModule::ShaderModule(ShaderModule&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_device, other.m_device);
        std::swap(shader, other.shader);
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_device, other.m_device);
        std::swap(shader, other.shader);

        return *this;
    }

    ShaderModule::~ShaderModule()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        vkDestroyShaderModule(m_device->getHandle(), m_handle, nullptr);
    }

    Program::Program(View<Device> device) : m_device(device) {}

    Program::Program(Program&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_shaderModules, other.m_shaderModules);
    }

    Program& Program::operator=(Program&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_shaderModules, other.m_shaderModules);

        return *this;
    }

    void Program::setShader(Shader shader) { m_shaderModules.emplace_back(m_device, std::move(shader)); }
} // namespace vzt
