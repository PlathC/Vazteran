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

		bool operator==(const Shader& other) const { return m_shaderStage == other.stage(); }

		VkShaderModuleCreateInfo getShaderModuleCreateInfo() const;
		vzt::ShaderStage         stage() const { return m_shaderStage; }

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

		~ShaderModule();

		VkShaderModule vkHandle() const { return m_vkHandle; }

	  private:
		VkShaderModule     m_vkHandle;
		const vzt::Device* m_device;
	};

	class Program
	{
		using ShaderList = std::unordered_map<vzt::ShaderStage, vzt::Shader>;

	  public:
		Program();

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		Program(Program&& other) noexcept;
		Program& operator=(Program&& other) noexcept;

		~Program();

		void              setShader(Shader shader);
		const ShaderList& getShaders() const { return m_shaders; }

		// Regenerate modules and stages
		void compile(const vzt::Device* const device);

		// Post Compiling informations
		const std::vector<VkPipelineShaderStageCreateInfo>& getPipelineStages() const { return m_pipelineShaderStages; }

	  private:
		const vzt::Device* m_device = nullptr;
		ShaderList         m_shaders;

		std::vector<vzt::ShaderModule>               m_shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> m_pipelineShaderStages;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_SHADER_HPP
