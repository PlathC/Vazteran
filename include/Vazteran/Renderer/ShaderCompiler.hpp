#ifndef VAZTERAN_SHADERCOMPILER_HPP
#define VAZTERAN_SHADERCOMPILER_HPP

#include <string>

#include <glslang/Public/ShaderLang.h>

#include "Vazteran/Backend/Vulkan/Shader.hpp"
#include "Vazteran/FileSystem/File.hpp"

namespace vzt
{
	enum class ShaderLanguage
	{
		GLSL,
		HLSL
	};

	class ShaderCompiler
	{
	  public:
		ShaderCompiler();

		void addIncludePath(Path path);

		std::vector<uint32_t> compile(const Path& path, const std::string& source, ShaderStage stage,
		                              bool optimize = true, ShaderLanguage language = ShaderLanguage::GLSL) const;

	  private:
		static std::atomic<bool>        IsInitialized;
		static std::atomic<std::size_t> InstanceCount;

		static ShaderStage        extensionToStage(std::string_view extension);
		static EShLanguage        toBackend(ShaderStage stage);
		static glslang::EShSource toBackend(ShaderLanguage stage);

		std::vector<Path> m_includePaths{};

		class Includer : public shaderc::CompileOptions::IncluderInterface
		{
		  public:
			shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type,
			                                   const char* requestingSource, size_t includeDepth) override;

			// Handles shaderc_include_result_release_fn callbacks.
			void ReleaseInclude(shaderc_include_result* data) override;

			~Includer() override = default;

		  private:
			static std::size_t hashPath(const Path& path);

			struct IncludedFile
			{
				shaderc_include_result result;

				std::string sourceName;
				std::string sourceContent;
			};

			std::unordered_map<std::size_t, IncludedFile> m_included;
		};
	};
} // namespace vzt

#endif // VAZTERAN_SHADERCOMPILER_HPP
