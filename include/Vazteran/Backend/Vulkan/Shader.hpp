#ifndef VAZTERAN_BACKEND_VULKAN_SHADER_HPP
#define VAZTERAN_BACKEND_VULKAN_SHADER_HPP

#include <optional>
#include <unordered_set>

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/GpuObjects.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	class Device;

	enum class ShaderStage
	{
		VertexShader                = VK_SHADER_STAGE_VERTEX_BIT,
		TesselationControlShader    = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		TesselationEvaluationShader = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		GeometryShader              = VK_SHADER_STAGE_GEOMETRY_BIT,
		FragmentShader              = VK_SHADER_STAGE_FRAGMENT_BIT,
		LastValue
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
		ShaderModule(const vzt::Device* const logicalDevice, VkShaderModuleCreateInfo createInfo);

		ShaderModule(const ShaderModule&) = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;

		ShaderModule(ShaderModule&& other) noexcept;
		ShaderModule& operator=(ShaderModule&& other) noexcept;

		VkShaderModule VkHandle() const { return m_vkHandle; }

		~ShaderModule();

	  private:
		VkShaderModule     m_vkHandle;
		const vzt::Device* m_device;
	};

	class Program
	{
	  public:
		Program(const vzt::Device* const device);

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		Program(Program&& other) noexcept;
		Program& operator=(Program&& other) noexcept;

		~Program();

		const std::unordered_map<vzt::ShaderStage, vzt::Shader>& Shaders() const { return m_shaders; };

		// Configuration
		void SetShader(Shader shader);

		// Regenerate modules and stages
		void Compile();

		// Post Compiling informations
		const std::vector<VkPipelineShaderStageCreateInfo>& PipelineStages() const { return m_pipelineShaderStages; }

	  private:
		const vzt::Device*                                m_device = nullptr;
		std::unordered_map<vzt::ShaderStage, vzt::Shader> m_shaders;

		std::vector<vzt::ShaderModule>               m_shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> m_pipelineShaderStages;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_SHADER_HPP
