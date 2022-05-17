#include <stdexcept>

#include "Vazteran/Renderer/ShaderCompiler.hpp"

namespace vzt
{
	std::atomic<bool>        ShaderCompiler::IsInitialized = false;
	std::atomic<std::size_t> ShaderCompiler::InstanceCount = 0;

	ShaderCompiler::ShaderCompiler()
	{
		// "Call this exactly once per process before using anything else"
		if (!IsInitialized)
		{
			IsInitialized = glslang::InitializeProcess();
		}

		InstanceCount++;
	}

	ShaderCompiler::~ShaderCompiler()
	{
		InstanceCount--;
		if (InstanceCount == 0)
		{
			glslang::FinalizeProcess();
			IsInitialized = false;
		}
	}

	std::vector<uint32_t> ShaderCompiler::compile(const Path& path, const std::string& source, ShaderStage stage,
	                                              bool optimize, ShaderLanguage language) const
	{
		ShHandle handle = ShConstructCompiler(toBackend(language), 0);
		// options.AddMacroDefinition("MY_DEFINE", "1");
		// if (optimize)
		// 	options.SetOptimizationLevel(shaderc_optimization_level_size);

		// options.SetIncluder(std::make_unique<Includer>());

		// shaderc::SpvCompilationResult module =
		//     compiler.CompileGlslToSpv(source, toShaderKind(stage), path.string().c_str(), options);

		// if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		// 	throw std::runtime_error(module.GetErrorMessage());

		// return {module.cbegin(), module.cend()};
	}

	ShaderStage extensionToStage(std::string_view extension)
	{
		if (extension == ".vert")
			return ShaderStage::Vertex;
		if (extension == ".frag")
			return ShaderStage::Fragment;
		if (extension == ".comp")
			return ShaderStage::Compute;
		if (extension == ".geom")
			return ShaderStage::Geometry;
		if (extension == ".tesc")
			return ShaderStage::TessellationControl;
		if (extension == ".tese")
			return ShaderStage::TessellationEvaluation;
		if (extension == ".rgen")
			return ShaderStage::RayGen;
		if (extension == ".rint")
			return ShaderStage::Intersection;
		if (extension == ".rahit")
			return ShaderStage::AnyHit;
		if (extension == ".rchit")
			return ShaderStage::ClosestHit;
		if (extension == ".rmiss")
			return ShaderStage::Miss;
		if (extension == ".rcall")
			return ShaderStage::Callable;
		if (extension == ".task")
			return ShaderStage::Task;
		if (extension == ".mesh")
			return ShaderStage::Mesh;

		throw std::runtime_error("Unknown extension");
	}

	EShLanguage ShaderCompiler::toBackend(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex: return EShLanguage::EShLangVertex;
		case ShaderStage::Fragment: return EShLanguage::EShLangFragment;
		case ShaderStage::Compute: return EShLanguage::EShLangCompute;
		case ShaderStage::Geometry: return EShLanguage::EShLangGeometry;
		case ShaderStage::TessellationControl: return EShLanguage::EShLangTessControl;
		case ShaderStage::TessellationEvaluation: return EShLanguage::EShLangTessEvaluation;
		case ShaderStage::RayGen: return EShLanguage::EShLangRayGen;
		case ShaderStage::AnyHit: return EShLanguage::EShLangAnyHit;
		case ShaderStage::ClosestHit: return EShLanguage::EShLangClosestHit;
		case ShaderStage::Miss: return EShLanguage::EShLangMiss;
		case ShaderStage::Intersection: return EShLanguage::EShLangIntersect;
		case ShaderStage::Callable: return EShLanguage::EShLangCallable;
		case ShaderStage::Task: return EShLanguage::EShLangTaskNV;
		case ShaderStage::Mesh: return EShLanguage::EShLangMeshNV;
		}

		throw std::runtime_error("Unknown shader type");
	}

	glslang::EShSource ShaderCompiler::toBackend(ShaderLanguage language)
	{
		switch (language)
		{
		case ShaderLanguage::GLSL: return glslang::EShSource::EShSourceGlsl;
		case ShaderLanguage::HLSL: return glslang::EShSource::EShSourceHlsl;
		}

		throw std::runtime_error("Unknown shader language");
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
