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
		ShaderLibrary() = default;
		~ShaderLibrary();

		const Shader& get(const Path& path, ShaderLanguage language = ShaderLanguage::GLSL);

		void reload();

	  private:
		struct ShaderSave;

		ShaderSave& getOrAdd(const Path& path, ShaderLanguage language);
		ShaderSave& add(std::size_t hash, const Path& path, ShaderStage stage, ShaderLanguage language);

		static std::size_t hashPath(const Path& path);
		static ShaderStage extensionToStage(std::string_view extension);

		using FileTime = std::filesystem::file_time_type;
		struct ShaderSave
		{
			Path           path;
			Shader         shader;
			ShaderLanguage language;
			FileTime       lastWriteTime;
		};

		ShaderCompiler                              m_compiler;
		std::unordered_map<std::size_t, ShaderSave> m_shaders;
	};
} // namespace vzt

#endif // VAZTERAN_SHADERLIBRARY_HPP
