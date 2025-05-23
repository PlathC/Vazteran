#include "vzt/Vulkan/Program.hpp"

#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    ShaderModule::ShaderModule(View<Device> device, Shader shader)
        : DeviceObject<VkShaderModule>(device), m_shader(std::move(shader))
    {
        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = m_shader.compiledSource.size() * sizeof(uint32_t);
        shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(m_shader.compiledSource.data());

        vkCheck(vkCreateShaderModule(m_device->getHandle(), &shaderModuleCreateInfo, nullptr, &m_handle),
                "Failed to create shader module.");
    }

    ShaderModule::ShaderModule(ShaderModule&& other) noexcept : DeviceObject<VkShaderModule>(std::move(other))
    {
        std::swap(m_shader, other.m_shader);
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept
    {
        std::swap(m_shader, other.m_shader);

        DeviceObject<VkShaderModule>::operator=(std::move(other));
        return *this;
    }

    ShaderModule::~ShaderModule()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        vkDestroyShaderModule(m_device->getHandle(), m_handle, nullptr);
    }

    Program::Program(View<Device> device) : m_device(device) {}

    Program::Program(View<Device> device, std::vector<Shader> shaders) : Program(device)
    {
        for (auto&& shader : shaders)
            setShader(std::move(shader));
    }

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

    ShaderGroup::ShaderGroup(View<Device> device) : m_device(device) {}

    ShaderGroup::ShaderGroup(ShaderGroup&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_shaders, other.m_shaders);
    }

    ShaderGroup& ShaderGroup::operator=(ShaderGroup&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_shaders, other.m_shaders);

        return *this;
    }

    void ShaderGroup::addShader(Shader shader, ShaderGroupType hitGroupType)
    {
        m_shaders.emplace_back(ShaderGroupShader{
            hitGroupType,
            ShaderModule(m_device, std::move(shader)),
        });
    }
} // namespace vzt
