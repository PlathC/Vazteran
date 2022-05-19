#ifndef VAZTERAN_SHADERCOMPILER_HPP
#define VAZTERAN_SHADERCOMPILER_HPP

#include <string>
#include <unordered_map>

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

		std::tuple<Shader, std::vector<Path>> compile(const Path& path, ShaderStage stage, bool optimize = true,
		                                              ShaderLanguage language = ShaderLanguage::GLSL) const;

	  private:
		static std::atomic<bool>        IsInitialized;
		static std::atomic<std::size_t> InstanceCount;

		static const TBuiltInResource DefaultBuiltInResource;

		static ShaderStage        extensionToStage(std::string_view extension);
		static EShLanguage        toBackend(ShaderStage stage);
		static glslang::EShSource toBackend(ShaderLanguage stage);

		std::vector<Path> m_includePaths{};

		class ShaderIncluder : public glslang::TShader::Includer
		{
		  public:
			using IncludeResult = glslang::TShader::Includer::IncludeResult;

			ShaderIncluder(Path root) : m_root(std::move(root)) {}

			IncludeResult* includeSystem(const char* headerName, const char* includerName,
			                             size_t inclusionDepth) override;
			IncludeResult* includeLocal(const char* headerName, const char* includerName,
			                            size_t inclusionDepth) override;

			// Handles shaderc_include_result_release_fn callbacks.
			void releaseInclude(IncludeResult* data) override;

			const std::vector<Path>& getIncludedList() const { return m_includedSave; }

			~ShaderIncluder() override = default;

		  private:
			static const std::size_t MaxDepth;
			static std::size_t       hashPath(const Path& path);

			struct DataKeeper
			{
				std::string sourceName;
				std::string sourceContent;
				std::size_t resultIndex;
			};

			Path                                        m_root;
			std::unordered_map<std::size_t, DataKeeper> m_included;
			std::vector<IncludeResult>                  m_resultsSave;
			std::vector<Path>                           m_includedSave;
		};
	};
} // namespace vzt

#endif // VAZTERAN_SHADERCOMPILER_HPP
