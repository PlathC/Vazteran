#ifndef VAZTERAN_VULKAN_SHADER_HPP
#define VAZTERAN_VULKAN_SHADER_HPP

#include <optional>
#include <unordered_set>

#include <vulkan/vulkan.h>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"

namespace vzt
{
	class Device;

	enum class ShaderStage
	{
		Vertex                 = VK_SHADER_STAGE_VERTEX_BIT,
		TessellationControl    = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		TessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		Geometry               = VK_SHADER_STAGE_GEOMETRY_BIT,
		Fragment               = VK_SHADER_STAGE_FRAGMENT_BIT,
		Compute                = VK_SHADER_STAGE_COMPUTE_BIT,
		RayGen                 = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
		AnyHit                 = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
		ClosestHit             = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
		Miss                   = VK_SHADER_STAGE_MISS_BIT_KHR,
		Intersection           = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
		Callable               = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
		Task                   = VK_SHADER_STAGE_TASK_BIT_NV,
		Mesh                   = VK_SHADER_STAGE_MESH_BIT_NV
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

	struct Shader
	{
		ShaderStage           stage;
		std::vector<uint32_t> compiledSource;
	};

	class ShaderModule
	{
	  public:
		ShaderModule(const vzt::Device* const logicalDevice, VkShaderModuleCreateInfo createInfo);

		ShaderModule(const ShaderModule&)            = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;

		ShaderModule(ShaderModule&& other);
		ShaderModule& operator=(ShaderModule&& other);

		~ShaderModule();

		VkShaderModule vkHandle() const { return m_vkHandle; }

	  private:
		VkShaderModule     m_vkHandle = VK_NULL_HANDLE;
		const vzt::Device* m_device   = nullptr;
	};

	class Program
	{
	  public:
		Program() = default;

		Program(const Program&)            = delete;
		Program& operator=(const Program&) = delete;

		Program(Program&& other) noexcept;
		Program& operator=(Program&& other) noexcept;

		~Program() = default;

		void setShader(const Shader& shader);

		// Regenerate modules and stages
		void compile(const vzt::Device* const device);
		bool isCompiled() const { return m_isCompiled; }

		// Post Compiling informations
		const std::vector<VkPipelineShaderStageCreateInfo>& getPipelineStages();

	  private:
		static VkShaderModuleCreateInfo getShaderModuleCreateInfo(const Shader& shader);

		using ShaderList = std::unordered_map<ShaderStage, VkShaderModuleCreateInfo>;
		ShaderList m_shaders;
		bool       m_isCompiled = false;

		std::vector<ShaderModule>                    m_shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> m_pipelineShaderStages;
	};
} // namespace vzt

#endif // VAZTERAN_VULKAN_SHADER_HPP
