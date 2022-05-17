#include "Vazteran/Renderer/ShaderLibrary.hpp"

namespace vzt
{
	ShaderLibrary::~ShaderLibrary()
	{
		m_shouldUpdate = false;
		if (m_updateThread.joinable())
			m_updateThread.join();
	}

	const Shader& ShaderLibrary::get(const Path& path, ShaderLanguage language, ShaderUpdatedCallback callback)
	{
		ShaderSave& save = getOrAdd(path, language);
		if (!callback)
		{
			save.callbackList.emplace_back(std::move(callback));
			if (!m_shouldUpdate)
				startThread();
		}

		return save.shader;
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

	void ShaderLibrary::startThread()
	{
		m_shouldUpdate = true;
		m_updateThread = std::thread([&] {
			while (m_shouldUpdate)
			{
				for (auto& save : m_shaders)
				{
					if (save.second.callbackList.empty())
						continue;

					if (std::filesystem::last_write_time(save.second.path) != save.second.lastWriteTime)
					{
						const ShaderStage stage = save.second.shader.stage;
						Shader shader = m_compiler.compile(save.second.path, stage, true, save.second.language);
						for (auto& callback : save.second.callbackList)
							callback(save.second.path, &save.second.shader);
						save.second.shader = std::move(shader);
					}
				}

				std::this_thread::sleep_for(m_threadSleepDuration);
			}
		});
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
