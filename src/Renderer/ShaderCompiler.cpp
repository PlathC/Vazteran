#include <stdexcept>

#include "Vazteran/Renderer/ShaderCompiler.hpp"

namespace vzt
{
	std::vector<uint32_t> ShaderCompiler::compile(const Path& path, const std::string& source, ShaderStage stage,
	                                              bool optimize) const
	{
		shaderc::Compiler       compiler;
		shaderc::CompileOptions options;

		// options.AddMacroDefinition("MY_DEFINE", "1");
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_size);

		options.SetIncluder(std::make_unique<Includer>());

		shaderc::SpvCompilationResult module =
		    compiler.CompileGlslToSpv(source, toShaderKind(stage), path.string().c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			throw std::runtime_error(module.GetErrorMessage());

		return {module.cbegin(), module.cend()};
	}

	shaderc_shader_kind ShaderCompiler::toShaderKind(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex: return shaderc_shader_kind::shaderc_vertex_shader; break;
		case ShaderStage::Fragment: return shaderc_shader_kind::shaderc_fragment_shader; break;
		case ShaderStage::Compute: return shaderc_shader_kind::shaderc_compute_shader; break;
		case ShaderStage::Geometry: return shaderc_shader_kind::shaderc_geometry_shader; break;
		case ShaderStage::TessellationControl: return shaderc_shader_kind::shaderc_tess_control_shader; break;
		case ShaderStage::TessellationEvaluation: return shaderc_shader_kind::shaderc_tess_evaluation_shader; break;
		case ShaderStage::RayGen: return shaderc_shader_kind::shaderc_raygen_shader; break;
		case ShaderStage::AnyHit: return shaderc_shader_kind::shaderc_anyhit_shader; break;
		case ShaderStage::ClosestHit: return shaderc_shader_kind::shaderc_closesthit_shader; break;
		case ShaderStage::Miss: return shaderc_shader_kind::shaderc_miss_shader; break;
		case ShaderStage::Intersection: return shaderc_shader_kind::shaderc_intersection_shader; break;
		case ShaderStage::Callable: return shaderc_shader_kind::shaderc_callable_shader; break;
		case ShaderStage::Task: return shaderc_shader_kind::shaderc_glsl_task_shader; break;
		case ShaderStage::Mesh: return shaderc_shader_kind::shaderc_glsl_mesh_shader; break;
		}

		throw std::runtime_error("Unknown shader type");
	}

	shaderc_include_result* ShaderCompiler::Includer::GetInclude(const char* requestedSource, shaderc_include_type type,
	                                                             const char* requestingSource, size_t includeDepth)
	{
		const Path requested{requestedSource};
		const Path requesting{requestingSource};

		const Path completePath = requesting / requested;

		IncludedFile include{};
		if (std::filesystem::exists(completePath))
		{
			include.sourceName    = completePath.string();
			include.sourceContent = vzt::readFile(completePath);
		}

		include.result.source_name        = include.sourceName.data();
		include.result.source_name_length = include.sourceName.size();
		include.result.content            = include.sourceContent.data();
		include.result.content_length     = include.sourceContent.size();

		const std::size_t hash = hashPath(completePath);
		m_included[hash]       = include;
		return &m_included[hash].result;
	}

	void ShaderCompiler::Includer::ReleaseInclude(shaderc_include_result* data)
	{
		m_included.erase(hashPath(Path{data->source_name}));
	}

	std::size_t ShaderCompiler::Includer::hashPath(const Path& path) { return std::filesystem::hash_value(path); }

} // namespace vzt
