#include "Vazteran/Renderer/ShaderLibrary.hpp"
#include "Vazteran/Core/Logger.hpp"

namespace vzt
{
	ShaderLibrary::~ShaderLibrary() = default;

	const Shader& ShaderLibrary::get(const Path& path, ShaderLanguage language)
	{
		ShaderSave& save = getOrAdd(path, language);
		return save.shader;
	}

	void ShaderLibrary::reload()
	{
		for (auto& [_, save] : m_shaders)
		{
			const auto currentTime = std::filesystem::last_write_time(save.path);
			if (currentTime > save.lastWriteTime)
			{
				save.lastWriteTime = currentTime;

				try
				{
					save.shader = m_compiler.compile(save.path, save.shader.stage, true, save.language);
				}
				catch (const std::exception& e)
				{
					VZT_INFO("Error while compiling {}: {}", save.path.string(), e.what());
				}
			}
		}
	}

	ShaderLibrary::ShaderSave& ShaderLibrary::getOrAdd(const Path& path, ShaderLanguage language)
	{
		const std::size_t hash = hashPath(path);

		const auto save = m_shaders.find(hash);
		if (save == m_shaders.end())
			return add(hash, path, extensionToStage(path.extension().string()), language);

		return save->second;
	}

	ShaderLibrary::ShaderSave& ShaderLibrary::add(std::size_t hash, const Path& path, ShaderStage stage,
	                                              ShaderLanguage language)
	{
		ShaderSave current{path, m_compiler.compile(path, stage, true, language), language};
		current.lastWriteTime = std::filesystem::last_write_time(path);
		m_shaders[hash]       = std::move(current);

		return m_shaders[hash];
	}

	std::size_t ShaderLibrary::hashPath(const Path& path) { return std::filesystem::hash_value(path); }
	ShaderStage ShaderLibrary::extensionToStage(std::string_view extension)
	{
		if (extension == ".vert")
			return ShaderStage::Vertex;
		if (extension == ".tesc")
			return ShaderStage::TessellationControl;
		if (extension == ".tese")
			return ShaderStage::TessellationEvaluation;
		if (extension == ".geom")
			return ShaderStage::Geometry;
		if (extension == ".frag")
			return ShaderStage::Fragment;
		if (extension == ".comp")
			return ShaderStage::Compute;
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

		throw std::runtime_error("Unrecognized shader stage");
	}

} // namespace vzt
