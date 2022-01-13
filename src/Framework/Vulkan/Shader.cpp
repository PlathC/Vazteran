#include <utility>

#include "Vazteran/Framework/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Framework/Vulkan/Shader.hpp"

namespace vzt
{
	Shader::Shader(const fs::path &compiled_file, vzt::ShaderStage shaderStage) : m_shaderStage(shaderStage)
	{
		m_compiledSource = vzt::ReadFile(compiled_file);
	}

	VkShaderModuleCreateInfo Shader::ShaderModuleCreateInfo() const
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = m_compiledSource.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(m_compiledSource.data());

		return shaderModuleCreateInfo;
	}

	void Shader::SetSamplerDescriptorSet(uint32_t binding)
	{
		for (auto &sampler : m_samplerDescriptorSets)
		{
			if (sampler.binding == binding)
			{
				return;
			}
		}

		// If this binding is not currently used
		m_samplerDescriptorSets.emplace_back(SamplerDescriptorSet{binding});
	}

	void Shader::SetUniformDescriptorSet(uint32_t binding, uint32_t size)
	{
		for (auto &uniform : m_uniformDescriptorSets)
		{
			if (uniform.binding == binding)
				uniform.size = size;
			return;
		}

		// If this binding is not currently used
		m_uniformDescriptorSets.emplace_back(SizedDescriptorSet{binding, size});
	}

	void Shader::AddPushConstant(uint32_t size)
	{
		m_pushConstants.emplace_back(size);
	}

	std::vector<std::pair<uint32_t, VkDescriptorType>> Shader::DescriptorTypes() const
	{
		std::vector<std::pair<uint32_t, VkDescriptorType>> descriptorTypes;
		for (const auto &samplerDescriptorSet : m_samplerDescriptorSets)
			descriptorTypes.emplace_back(samplerDescriptorSet.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		for (const auto &uniformDescriptorSet : m_uniformDescriptorSets)
			descriptorTypes.emplace_back(uniformDescriptorSet.binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

		return descriptorTypes;
	}

	std::vector<VkPushConstantRange> Shader::PushConstants() const
	{
		std::vector<VkPushConstantRange> pushConstants;
		pushConstants.reserve(m_pushConstants.size());

		uint32_t offsetAccumulation = 0;
		for (auto pushConstantSize : m_pushConstants)
		{
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = static_cast<VkShaderStageFlagBits>(m_shaderStage);
			pushConstantRange.offset = offsetAccumulation;
			pushConstantRange.size = pushConstantSize;
			pushConstants.emplace_back(pushConstantRange);
			offsetAccumulation += pushConstantSize;
		}

		return pushConstants;
	}

	ShaderModule::ShaderModule(vzt::LogicalDevice *logicalDevice, VkShaderModuleCreateInfo createInfo)
	    : m_logicalDevice(logicalDevice)
	{
		if (vkCreateShaderModule(m_logicalDevice->VkHandle(), &createInfo, nullptr, &m_vkHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!");
		}
	}

	ShaderModule::ShaderModule(ShaderModule &&other) noexcept
	{
		m_vkHandle = std::exchange(other.m_vkHandle, static_cast<decltype(m_vkHandle)>(VK_NULL_HANDLE));
		std::swap(m_logicalDevice, other.m_logicalDevice);
	}

	ShaderModule &ShaderModule::operator=(ShaderModule &&other) noexcept
	{
		std::swap(m_vkHandle, other.m_vkHandle);
		std::swap(m_logicalDevice, other.m_logicalDevice);

		return *this;
	}

	ShaderModule::~ShaderModule()
	{
		if (m_vkHandle != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(m_logicalDevice->VkHandle(), m_vkHandle, nullptr);
		}
	}
} // namespace vzt
