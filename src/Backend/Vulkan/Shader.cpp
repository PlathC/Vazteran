#include <utility>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/Shader.hpp"
#include "Vazteran/FileSystem/File.hpp"

namespace vzt
{
	ShaderModule::ShaderModule(const vzt::Device* const device, VkShaderModuleCreateInfo createInfo) : m_device(device)
	{
		if (vkCreateShaderModule(m_device->vkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shader module!");
	}

	ShaderModule::ShaderModule(ShaderModule&& other)
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_device, other.m_device);
	}

	ShaderModule& ShaderModule::operator=(ShaderModule&& other)
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_device, other.m_device);

		return *this;
	}

	ShaderModule::~ShaderModule()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(m_device->vkHandle(), m_vkHandle, nullptr);
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

	Program::Program(Program&& other) noexcept
	{
		std::swap(m_shaders, other.m_shaders);
		std::swap(m_isCompiled, other.m_isCompiled);
		std::swap(m_shaderModules, other.m_shaderModules);
		std::swap(m_pipelineShaderStages, other.m_pipelineShaderStages);
	}

	Program& Program::operator=(Program&& other) noexcept
	{
		std::swap(m_shaders, other.m_shaders);
		std::swap(m_isCompiled, other.m_isCompiled);
		std::swap(m_shaderModules, other.m_shaderModules);
		std::swap(m_pipelineShaderStages, other.m_pipelineShaderStages);

		return *this;
	}

	void Program::setShader(const Shader& shader)
	{
		m_shaders.emplace(shader.stage, getShaderModuleCreateInfo(shader));
	}

	void Program::compile(const vzt::Device* const device)
	{
		m_shaderModules.clear();
		m_pipelineShaderStages.clear();
		for (const auto& stage : m_shaders)
		{
			ShaderModule                    shaderModule{device, stage.second};
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
			shaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageCreateInfo.stage  = static_cast<VkShaderStageFlagBits>(stage.first);
			shaderStageCreateInfo.module = shaderModule.vkHandle();
			shaderStageCreateInfo.pName  = "main";

			m_shaderModules.emplace_back(std::move(shaderModule));
			m_pipelineShaderStages.emplace_back(shaderStageCreateInfo);
		}
	}

	const std::vector<VkPipelineShaderStageCreateInfo>& Program::getPipelineStages() { return m_pipelineShaderStages; }

	VkShaderModuleCreateInfo Program::getShaderModuleCreateInfo(const Shader& shader)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = shader.compiledSource.size();
		shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(shader.compiledSource.data());

		return shaderModuleCreateInfo;
	}

} // namespace vzt
