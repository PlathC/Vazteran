#ifndef VAZTERAN_SHADERLIBRARY_HPP
#define VAZTERAN_SHADERLIBRARY_HPP

#include <chrono>
#include <thread>
#include <unordered_map>

#include "Vazteran/Backend/Vulkan/Shader.hpp"
#include "Vazteran/FileSystem/File.hpp"
#include "Vazteran/Renderer/ShaderCompiler.hpp"

namespace vzt
{
	class ShaderLibrary
	{
	  public:
		using ShaderUpdatedCallback = std::function<void(const Path& /* path */, const Shader*)>;

		ShaderLibrary() = default;
		~ShaderLibrary();

		const Shader& get(const Path& path);
		const Shader& get(const Path& path, ShaderUpdatedCallback callback);

	  private:
		struct ShaderSave;

		ShaderSave& getOrAdd(const Path& path);
		ShaderSave& add(std::size_t hash, const Path& path, ShaderStage stage);

		void startThread();

		static std::size_t hashPath(const Path& path);
		static ShaderStage extensionToStage(std::string_view extension);

		using FileTime = std::filesystem::file_time_type;
		struct ShaderSave
		{
			Path                               path;
			Shader                             shader;
			FileTime                           lastWriteTime;
			std::vector<ShaderUpdatedCallback> callbackList;
		};

		ShaderCompiler                              m_compiler;
		std::unordered_map<std::size_t, ShaderSave> m_shaders;

		std::thread       m_updateThread;
		std::atomic<bool> m_shouldUpdate = false;

		const std::chrono::duration<float, std::milli>& m_threadSleepDuration = std::chrono::milliseconds(1000);
	};
} // namespace vzt

#endif // VAZTERAN_SHADERLIBRARY_HPP
