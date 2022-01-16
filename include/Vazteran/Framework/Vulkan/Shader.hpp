#ifndef VAZTERAN_FRAMEWORK_VULKAN_SHADER_HPP
#define VAZTERAN_FRAMEWORK_VULKAN_SHADER_HPP

#include <optional>
#include <unordered_set>

#include <vulkan/vulkan.h>

#include "Vazteran/Core/Utils.hpp"
#include "Vazteran/Framework/Vulkan/Buffer.hpp"
#include "Vazteran/Framework/Vulkan/Descriptor.hpp"
#include "Vazteran/Framework/Vulkan/GpuObjects.hpp"
#include "Vazteran/Framework/Vulkan/ImageUtils.hpp"

namespace vzt
{
	class Device;

	enum class ShaderStage
	{
		VertexShader                = VK_SHADER_STAGE_VERTEX_BIT,
		TesselationControlShader    = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		TesselationEvaluationShader = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		GeometryShader              = VK_SHADER_STAGE_GEOMETRY_BIT,
		FragmentShader              = VK_SHADER_STAGE_FRAGMENT_BIT
	};

	struct DescriptorSet
	{
		uint32_t binding;
	};

	struct SamplerDescriptorSet : public DescriptorSet
	{
	};

	struct SizedDescriptorSet : public DescriptorSet
	{
		uint32_t size;
	};

	class Shader
	{
	  public:
		Shader(const fs::path& compiled_file, vzt::ShaderStage shaderStage);

		VkShaderModuleCreateInfo ShaderModuleCreateInfo() const;
		vzt::ShaderStage         Stage() const { return m_shaderStage; }

		void SetSamplerDescriptorSet(uint32_t binding);
		void SetUniformDescriptorSet(uint32_t binding, uint32_t size);
		void AddPushConstant(uint32_t size);

		std::vector<SamplerDescriptorSet> SamplerDescriptorSets() const { return m_samplerDescriptorSets; }
		std::vector<SizedDescriptorSet>   UniformDescriptorSets() const { return m_uniformDescriptorSets; }
		std::vector<std::pair<uint32_t, vzt::DescriptorType>> DescriptorTypes() const;
		std::vector<VkPushConstantRange>                      PushConstants() const;

		bool operator==(const Shader& other) const { return m_shaderStage == other.Stage(); }

	  private:
		std::vector<char>                 m_compiledSource;
		vzt::ShaderStage                  m_shaderStage;
		std::vector<SamplerDescriptorSet> m_samplerDescriptorSets{};
		std::vector<SizedDescriptorSet>   m_uniformDescriptorSets{};
		std::vector<uint32_t>             m_pushConstants{};
	};

	class ShaderModule
	{
	  public:
		ShaderModule(vzt::Device* logicalDevice, VkShaderModuleCreateInfo createInfo);

		ShaderModule(const ShaderModule&) = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;

		ShaderModule(ShaderModule&& other) noexcept;
		ShaderModule& operator=(ShaderModule&& other) noexcept;

		VkShaderModule VkHandle() const { return m_vkHandle; }

		~ShaderModule();

	  private:
		VkShaderModule m_vkHandle;
		vzt::Device*   m_device;
	};

	class Program
	{
	  public:
		Program(vzt::Device* device);

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		Program(Program&& other) noexcept;
		Program& operator=(Program&& other) noexcept;

		~Program();

		std::vector<vzt::DescriptorType>                         DescriptorTypes() const;
		const std::unordered_map<vzt::ShaderStage, vzt::Shader>& Shaders() const { return m_shaders; };

		// Configuration
		void SetShader(Shader shader);

		// Regenerate modules and stages
		void Compile();

		// Post Compiling informations
		const std::vector<VkPipelineShaderStageCreateInfo>& PipelineStages() const { return m_pipelineShaderStages; }
		const VkDescriptorSetLayout& DescriptorSetLayout() const { return m_descriptorSetLayout; }

	  private:
		vzt::Device*                                                                m_device = nullptr;
		std::unordered_map<vzt::ShaderStage, vzt::Shader>                           m_shaders;
		std::unordered_map<uint32_t, std::pair<VkShaderStageFlags, DescriptorType>> m_descriptors;

		std::vector<vzt::ShaderModule>               m_shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> m_pipelineShaderStages;
		VkDescriptorSetLayout                        m_descriptorSetLayout = VK_NULL_HANDLE;
	};
} // namespace vzt

#endif // VAZTERAN_FRAMEWORK_VULKAN_SHADER_HPP
