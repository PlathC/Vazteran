#include <utility>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/Shader.hpp"

namespace vzt
{
	Shader::Shader(const fs::path& compiled_file, vzt::ShaderStage shaderStage) : m_shaderStage(shaderStage)
	{
		m_compiledSource = vzt::ReadFile(compiled_file);
	}

	VkShaderModuleCreateInfo Shader::getShaderModuleCreateInfo() const
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = m_compiledSource.size();
		shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(m_compiledSource.data());

		return shaderModuleCreateInfo;
	}

	ShaderModule::ShaderModule(const vzt::Device* const device, VkShaderModuleCreateInfo createInfo) : m_device(device)
	{
		if (vkCreateShaderModule(m_device->VkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!");
		}
	}

	ShaderModule::ShaderModule(ShaderModule&& other) noexcept
	{
		m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		std::swap(m_device, other.m_device);
	}

	ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_device, other.m_device);

		return *this;
	}

	ShaderModule::~ShaderModule()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(m_device->VkHandle(), m_vkHandle, nullptr);
		}
	}

	Program::Program(const vzt::Device* const device) : m_device(device) {}

	Program::Program(Program&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_shaders, other.m_shaders);
		std::swap(m_shaderModules, other.m_shaderModules);
		std::swap(m_pipelineShaderStages, other.m_pipelineShaderStages);
	}

	Program& Program::operator=(Program&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_shaders, other.m_shaders);
		std::swap(m_shaderModules, other.m_shaderModules);
		std::swap(m_pipelineShaderStages, other.m_pipelineShaderStages);
		return *this;
	}

	Program::~Program() = default;

	void Program::setShader(Shader shader)
	{
		const auto stage = shader.stage();
		m_shaders.emplace(stage, std::move(shader));
	}

	void Program::compile()
	{
		m_shaderModules.clear();
		m_pipelineShaderStages.clear();
		for (const auto& stage : m_shaders)
		{
			const auto& shader = stage.second;
			auto        module = ShaderModule(m_device, shader.getShaderModuleCreateInfo());

			VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
			shaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageCreateInfo.stage  = static_cast<VkShaderStageFlagBits>(shader.stage());
			shaderStageCreateInfo.module = module.vkHandle();
			shaderStageCreateInfo.pName  = "main";

			m_shaderModules.emplace_back(std::move(module));
			m_pipelineShaderStages.emplace_back(shaderStageCreateInfo);
		}
	}
} // namespace vzt
