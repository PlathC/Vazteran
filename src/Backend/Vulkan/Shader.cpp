#include <utility>

#include "Vazteran/Backend/Vulkan/Device.hpp"
#include "Vazteran/Backend/Vulkan/Shader.hpp"

namespace vzt
{
	Shader::Shader(const fs::path& compiled_file, vzt::ShaderStage shaderStage) : m_shaderStage(shaderStage)
	{
		m_compiledSource = vzt::ReadFile(compiled_file);
	}

	VkShaderModuleCreateInfo Shader::ShaderModuleCreateInfo() const
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = m_compiledSource.size();
		shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(m_compiledSource.data());

		return shaderModuleCreateInfo;
	}

	void Shader::SetSamplerDescriptorSet(uint32_t binding)
	{
		for (auto& sampler : m_samplerDescriptorSets)
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
		for (auto& uniform : m_uniformDescriptorSets)
		{
			if (uniform.binding == binding)
				uniform.size = size;
			return;
		}

		// If this binding is not currently used
		m_uniformDescriptorSets.emplace_back(SizedDescriptorSet{binding, size});
	}

	void Shader::AddPushConstant(uint32_t size) { m_pushConstants.emplace_back(size); }

	std::vector<std::pair<uint32_t, vzt::DescriptorType>> Shader::DescriptorTypes() const
	{
		std::vector<std::pair<uint32_t, vzt::DescriptorType>> descriptorTypes;
		descriptorTypes.reserve(m_samplerDescriptorSets.size() + m_uniformDescriptorSets.size());
		for (const auto& samplerDescriptorSet : m_samplerDescriptorSets)
		{
			descriptorTypes.emplace_back(samplerDescriptorSet.binding, vzt::DescriptorType::CombinedSampler);
		}

		for (const auto& uniformDescriptorSet : m_uniformDescriptorSets)
		{
			descriptorTypes.emplace_back(uniformDescriptorSet.binding, vzt::DescriptorType::UniformBuffer);
		}

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
			pushConstantRange.offset     = offsetAccumulation;
			pushConstantRange.size       = pushConstantSize;
			pushConstants.emplace_back(pushConstantRange);
			offsetAccumulation += pushConstantSize;
		}

		return pushConstants;
	}

	ShaderModule::ShaderModule(vzt::Device* device, VkShaderModuleCreateInfo createInfo) : m_device(device)
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

	Program::Program(vzt::Device* device) : m_device(device) {}

	Program::Program(Program&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_shaders, other.m_shaders);
		std::swap(m_descriptors, other.m_descriptors);
		std::swap(m_shaderModules, other.m_shaderModules);
		std::swap(m_pipelineShaderStages, other.m_pipelineShaderStages);
		std::swap(m_descriptorSetLayout, other.m_descriptorSetLayout);
	}

	Program& Program::operator=(Program&& other) noexcept
	{
		std::swap(m_device, other.m_device);
		std::swap(m_shaders, other.m_shaders);
		std::swap(m_descriptors, other.m_descriptors);
		std::swap(m_shaderModules, other.m_shaderModules);
		std::swap(m_pipelineShaderStages, other.m_pipelineShaderStages);
		std::swap(m_descriptorSetLayout, other.m_descriptorSetLayout);
		return *this;
	}

	Program::~Program()
	{
		if (m_descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(m_device->VkHandle(), m_descriptorSetLayout, nullptr);
			m_descriptorSetLayout = VK_NULL_HANDLE;
		}
	}

	void Program::SetShader(Shader shader)
	{
		for (const auto descriptor : shader.DescriptorTypes())
		{
			if (m_descriptors.find(descriptor.first) == m_descriptors.end())
			{
				m_descriptors[descriptor.first] =
				    std::make_pair(static_cast<VkShaderStageFlags>(shader.Stage()), descriptor.second);
			}
			else
			{
				m_descriptors[descriptor.first].first |= static_cast<VkShaderStageFlags>(shader.Stage());
			}
		}

		const auto stage = shader.Stage();
		m_shaders.emplace(stage, std::move(shader));
	}

	std::vector<DescriptorType> Program::DescriptorTypes() const
	{
		std::vector<DescriptorType> descriptorTypes;
		for (const auto& stage : m_shaders)
		{
			const auto& shader = stage.second;
			auto        types  = shader.DescriptorTypes();
			for (const auto& type : types)
				descriptorTypes.emplace_back(type.second);
		}

		return descriptorTypes;
	}

	void Program::Compile()
	{
		if (m_descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(m_device->VkHandle(), m_descriptorSetLayout, nullptr);
			m_descriptorSetLayout = VK_NULL_HANDLE;
		}

		m_shaderModules.clear();
		m_pipelineShaderStages.clear();
		for (const auto& stage : m_shaders)
		{
			const auto& shader = stage.second;
			auto        module = ShaderModule(m_device, shader.ShaderModuleCreateInfo());

			VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
			shaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageCreateInfo.stage  = static_cast<VkShaderStageFlagBits>(shader.Stage());
			shaderStageCreateInfo.module = module.VkHandle();
			shaderStageCreateInfo.pName  = "main";

			m_shaderModules.emplace_back(std::move(module));
			m_pipelineShaderStages.emplace_back(shaderStageCreateInfo);
		}

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.reserve(m_descriptors.size());
		for (const auto& descriptor : m_descriptors)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding            = descriptor.first;
			layoutBinding.descriptorCount    = 1;
			layoutBinding.pImmutableSamplers = nullptr; // Optional
			layoutBinding.stageFlags         = descriptor.second.first;
			layoutBinding.descriptorType = static_cast<VkDescriptorType>(vzt::ToUnderlying(descriptor.second.second));
			bindings.emplace_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings    = bindings.data();

		if (vkCreateDescriptorSetLayout(m_device->VkHandle(), &layoutInfo, nullptr, &m_descriptorSetLayout) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
	}
} // namespace vzt
