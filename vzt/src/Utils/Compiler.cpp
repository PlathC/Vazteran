#include "vzt/Utils/Compiler.hpp"

#include <unordered_map>

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>

#include "vzt/Core/Logger.hpp"
#include "vzt/Vulkan/Instance.hpp"

namespace vzt
{
    std::atomic<bool> Compiler::IsInitialized = false;

    std::size_t hashPath(const Path& path) { return std::filesystem::hash_value(path); }

    class ShaderIncluder : public glslang::TShader::Includer
    {
      public:
        using IncludeResult = glslang::TShader::Includer::IncludeResult;

        ShaderIncluder(Path root) : m_root(std::move(root)) {}
        ~ShaderIncluder() override = default;

        IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t) override
        {
            const Path requested{headerName};
            const Path requesting{includerName};
            const Path completePath = requesting / requested;

            std::string source{};
            if (std::filesystem::exists(completePath))
            {
                m_includedSave.emplace_back(completePath);
                source = vzt::readFile(completePath);
            }
            else
            {
                logger::error("Can't find {}!", completePath.string());
            }

            const std::size_t hash = hashPath(completePath);

            DataKeeper keeper{source.empty() ? "" : requested.string(), source, m_resultsSave.size()};

            m_resultsSave.emplace_back(std::make_unique<IncludeResult>( //
                keeper.sourceName, keeper.sourceContent.c_str(), keeper.sourceContent.size(), nullptr));
            m_included[hash] = std::move(keeper);

            return m_resultsSave.back().get();
        }

        IncludeResult* includeLocal(const char* headerName, const char*, size_t) override
        {
            const Path requested{headerName};
            const Path completePath = m_root / requested;

            std::string source{};
            if (std::filesystem::exists(completePath))
            {
                m_includedSave.emplace_back(completePath);
                source = vzt::readFile(completePath);
            }

            const std::size_t hash = hashPath(completePath);
            DataKeeper        keeper{requested.string(), source, m_resultsSave.size()};

            m_resultsSave.emplace_back(std::make_unique<IncludeResult>( //
                keeper.sourceName, keeper.sourceContent.c_str(), keeper.sourceContent.size(), nullptr));
            m_included[hash] = std::move(keeper);

            return m_resultsSave.back().get();
        }

        // Handles shaderc_include_result_release_fn callbacks.
        void releaseInclude(IncludeResult* data) override { m_included.erase(hashPath(Path{data->headerName})); }

        const std::vector<Path>& getIncludedList() const { return m_includedSave; }

      private:
        static const std::size_t MaxDepth;

        struct DataKeeper
        {
            std::string sourceName;
            std::string sourceContent;
            std::size_t resultIndex;
        };

        Path                                        m_root;
        std::unordered_map<std::size_t, DataKeeper> m_included;
        std::vector<std::unique_ptr<IncludeResult>> m_resultsSave;
        std::vector<Path>                           m_includedSave;
    };

    // From: https://github.com/KhronosGroup/glslang/blob/master/StandAlone/ResourceLimits.cpp#L44
    const TBuiltInResource DefaultTBuiltInResource = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
        /* .maxMeshOutputVerticesNV = */ 256,
        /* .maxMeshOutputPrimitivesNV = */ 512,
        /* .maxMeshWorkGroupSizeX_NV = */ 32,
        /* .maxMeshWorkGroupSizeY_NV = */ 1,
        /* .maxMeshWorkGroupSizeZ_NV = */ 1,
        /* .maxTaskWorkGroupSizeX_NV = */ 32,
        /* .maxTaskWorkGroupSizeY_NV = */ 1,
        /* .maxTaskWorkGroupSizeZ_NV = */ 1,
        /* .maxMeshViewCountNV = */ 4,
        /* .maxMeshOutputVerticesEXT = */ 256,
        /* .maxMeshOutputPrimitivesEXT = */ 256,
        /* .maxMeshWorkGroupSizeX_EXT = */ 128,
        /* .maxMeshWorkGroupSizeY_EXT = */ 128,
        /* .maxMeshWorkGroupSizeZ_EXT = */ 128,
        /* .maxTaskWorkGroupSizeX_EXT = */ 128,
        /* .maxTaskWorkGroupSizeY_EXT = */ 128,
        /* .maxTaskWorkGroupSizeZ_EXT = */ 128,
        /* .maxMeshViewCountEXT = */ 4,
        /* .maxDualSourceDrawBuffersEXT = */ 1,

        /* .limits = */
        {
            /* .nonInductiveForLoops = */ 1,
            /* .whileLoops = */ 1,
            /* .doWhileLoops = */ 1,
            /* .generalUniformIndexing = */ 1,
            /* .generalAttributeMatrixVectorIndexing = */ 1,
            /* .generalVaryingIndexing = */ 1,
            /* .generalSamplerIndexing = */ 1,
            /* .generalVariableIndexing = */ 1,
            /* .generalConstantMatrixVectorIndexing = */ 1,
        }};

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

    EShLanguage toBackend(ShaderStage stage)
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

    glslang::EShTargetClientVersion toBackend(VulkanVersion version)
    {
        switch (version)
        {
        case VulkanVersion::V1_0: return glslang::EShTargetClientVersion::EShTargetVulkan_1_0; break;
        case VulkanVersion::V1_1: return glslang::EShTargetClientVersion::EShTargetVulkan_1_1; break;
        case VulkanVersion::V1_2: return glslang::EShTargetClientVersion::EShTargetVulkan_1_2; break;
        case VulkanVersion::V1_3: return glslang::EShTargetClientVersion::EShTargetVulkan_1_3; break;
        }

        throw std::runtime_error("Unsupported Vulkan version");
    }

    glslang::EShSource toBackend(ShadingLanguage language)
    {
        switch (language)
        {
        case ShadingLanguage::GLSL: return glslang::EShSource::EShSourceGlsl;
        case ShadingLanguage::HLSL: return glslang::EShSource::EShSourceHlsl;
        }

        throw std::runtime_error("Unknown shader language");
    }

    Compiler::Compiler(View<Instance> instance) : m_instance(instance)
    {
        // "Call this exactly once per process before using anything else"
        if (!IsInitialized)
        {
            IsInitialized = glslang::InitializeProcess();
            if (!IsInitialized)
                throw std::runtime_error("Failed to initialize glslang");

            std::atexit(glslang::FinalizeProcess);
        }
    }

    Shader Compiler::compile(const Path& path, ShaderStage stage, ShadingLanguage language, bool optimize) const
    {
        const EShLanguage shaderStage = toBackend(stage);
        glslang::TShader  shader{shaderStage};

        std::string       code    = readFile(path);
        const char* const rawCode = code.data();
        shader.setStrings(&rawCode, 1);

        // TODO: Make this dynamic based on renderer
        glslang::EShClient              clientType     = glslang::EShClientVulkan;
        glslang::EShTargetClientVersion clientVersion  = toBackend(m_instance->getAPIVersion());
        constexpr int                   DefaultVersion = 450;

        shader.setEntryPoint("main");
        shader.setEnvInput(toBackend(language), toBackend(stage), clientType, DefaultVersion);
        shader.setEnvClient(clientType, clientVersion);
        shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetSpv_1_4);

        constexpr EProfile DefaultProfile = EProfile::ECoreProfile;

        std::string    preprocessed{};
        ShaderIncluder includer{path.parent_path()};
        if (!shader.parse(&DefaultTBuiltInResource, DefaultVersion, DefaultProfile, false, true,
                          EShMessages::EShMsgDefault, includer))
        {
            logger::debug("[COMPILER] {}", shader.getInfoDebugLog());
            logger::error("[COMPILER] {}", shader.getInfoLog());
            throw std::runtime_error(fmt::format("Failed to parse {}", path.string()));
        }

        glslang::TProgram program{};
        program.addShader(&shader);
        if (!program.link(EShMsgDefault))
        {
            const char* debugLog = shader.getInfoDebugLog();
            if (debugLog)
                logger::debug(debugLog);

            const char* infoLog = shader.getInfoLog();
            if (infoLog)
                logger::error(infoLog);
            throw std::runtime_error(fmt::format("Failed to link {}", path.string()));
        }

        spv::SpvBuildLogger       buildLogger{};
        std::vector<unsigned int> spirvData{};
        glslang::SpvOptions       options{};
        options.disableOptimizer = !optimize;
        options.validate         = true;

#ifndef _NDEBUG
        options.generateDebugInfo = true;
        options.stripDebugInfo    = true;
#endif // _NDEBUG

        glslang::GlslangToSpv(*program.getIntermediate(shaderStage), spirvData, &buildLogger, &options);

        const std::string messages = buildLogger.getAllMessages();
        if (!messages.empty())
            logger::debug("[COMPILER] {}", messages);

        return {stage, std::vector<uint32_t>{spirvData.begin(), spirvData.end()}};
    }

    Shader Compiler::compile(const Path& path, ShaderStage stage, ShadingLanguage language) const
    {
#ifndef NDEBUG
        constexpr bool WithOptimization = false;
#else
        constexpr bool WithOptimization = true;
#endif // NDEBUG

        return compile(path, stage, language, WithOptimization);
    }
} // namespace vzt
