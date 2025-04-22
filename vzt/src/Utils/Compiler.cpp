#include "vzt/Utils/Compiler.hpp"

#include <unordered_map>

#include <slang/slang.h>

#include "vzt/Core/Assert.hpp"
#include "vzt/Core/Logger.hpp"
#include "vzt/Vulkan/Instance.hpp"

#include <slang/slang-com-ptr.h>

namespace vzt
{
    struct Compiler::Implementation
    {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        Slang::ComPtr<slang::ISession>       session;
    };

    ShaderStage toShaderStage(SlangStage stage)
    {
        switch (stage)
        {
        case SLANG_STAGE_VERTEX: return ShaderStage::Vertex;
        case SLANG_STAGE_FRAGMENT: return ShaderStage::Fragment;
        case SLANG_STAGE_COMPUTE: return ShaderStage::Compute;
        case SLANG_STAGE_GEOMETRY: return ShaderStage::Geometry;
        case SLANG_STAGE_HULL: return ShaderStage::TessellationControl;
        case SLANG_STAGE_DOMAIN: return ShaderStage::TessellationEvaluation;
        case SLANG_STAGE_RAY_GENERATION: return ShaderStage::RayGen;
        case SLANG_STAGE_ANY_HIT: return ShaderStage::AnyHit;
        case SLANG_STAGE_CLOSEST_HIT: return ShaderStage::ClosestHit;
        case SLANG_STAGE_MISS: return ShaderStage::Miss;
        case SLANG_STAGE_INTERSECTION: return ShaderStage::Intersection;
        case SLANG_STAGE_CALLABLE: return ShaderStage::Callable;
        case SLANG_STAGE_AMPLIFICATION: return ShaderStage::Task;
        case SLANG_STAGE_MESH: return ShaderStage::Mesh; ;
        case SLANG_STAGE_NONE:
        case SLANG_STAGE_COUNT: break;
        }

        VZT_NOT_REACHED();
        return ShaderStage::All;
    }

    Compiler::~Compiler() = default;

    Compiler::Compiler(View<Instance> instance, const std::vector<vzt::Path>& includeDirectories) : m_instance(instance)
    {
        m_implementation = std::make_unique<Implementation>();

        slang::createGlobalSession(m_implementation->globalSession.writeRef());

        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc  target      = {
                  .format  = SLANG_SPIRV,
                  .profile = m_implementation->globalSession->findProfile("spirv_1_5"),
                  .flags   = 0,
        };

        sessionDesc.targets     = &target;
        sessionDesc.targetCount = 1;

        std::vector<slang::CompilerOptionEntry> compilerOptions = {
            {slang::CompilerOptionName::EmitSpirvDirectly, //
             {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
            {slang::CompilerOptionName::Optimization,
             {slang::CompilerOptionValueKind::Int, SLANG_OPTIMIZATION_LEVEL_DEFAULT, 0, nullptr, nullptr}},
            {slang::CompilerOptionName::DebugInformation,
             {
                 slang::CompilerOptionValueKind::Int,
#ifndef NDEBUG
                 SLANG_DEBUG_INFO_LEVEL_MAXIMAL,
#else
                 SLANG_DEBUG_INFO_LEVEL_NONE,
#endif // NDEBUG
                 0,
                 nullptr,
                 nullptr,
             }},
        };
        sessionDesc.compilerOptionEntries    = compilerOptions.data();
        sessionDesc.compilerOptionEntryCount = static_cast<uint32_t>(compilerOptions.size());

        std::vector<const char*> searchPaths;
        std::transform(begin(includeDirectories), end(includeDirectories), std::back_inserter(searchPaths),
                       [](const vzt::Path& path) { return path.c_str(); });

        sessionDesc.searchPaths     = searchPaths.data();
        sessionDesc.searchPathCount = static_cast<SlangInt>(searchPaths.size());

        m_implementation->globalSession->createSession(sessionDesc, m_implementation->session.writeRef());
    }

    Shader Compiler::operator()(const Path& path, const std::string& entryPoint) const
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        slang::IModule*             module;
        {
            module = m_implementation->session->loadModule(path.c_str(), diagnostics.writeRef());
            if (!module)
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
                std::abort();
            }
        }

        int32_t entryPointCount = module->getDefinedEntryPointCount();

        bool foundEntryPoint = false;
        for (int32_t i = 0; i < entryPointCount; ++i)
        {
            Slang::ComPtr<slang::IEntryPoint> iEntryPoint;
            module->getDefinedEntryPoint(i, iEntryPoint.writeRef());

            foundEntryPoint |= iEntryPoint->getFunctionReflection()->getName() == entryPoint;
        }
        VZT_ASSERT(foundEntryPoint);

        Slang::ComPtr<slang::IEntryPoint> iEntryPoint;
        module->findEntryPointByName(entryPoint.c_str(), iEntryPoint.writeRef());

        Slang::ComPtr<slang::IComponentType> composedProgram;
        {
            std::vector<slang::IComponentType*> componentTypes = {module, iEntryPoint};

            if (SLANG_FAILED(m_implementation->session->createCompositeComponentType( //
                    componentTypes.data(), static_cast<SlangInt>(componentTypes.size()), composedProgram.writeRef(),
                    diagnostics.writeRef())))
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
                std::abort();
            }
        }

        Slang::ComPtr<slang::IBlob> kernelBlob;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            if (SLANG_FAILED(composedProgram->getEntryPointCode(0, 0, kernelBlob.writeRef(), diagnostics.writeRef())))
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
                std::abort();
            }
        }

        slang::ProgramLayout*        programLayout = composedProgram->getLayout();
        slang::EntryPointReflection* reflection    = programLayout->findEntryPointByName(entryPoint.c_str());

        Shader shader = Shader(toShaderStage(reflection->getStage()), {});
        shader.compiledSource.resize(kernelBlob->getBufferSize() / sizeof(uint32_t));
        std::memcpy(shader.compiledSource.data(), kernelBlob->getBufferPointer(), kernelBlob->getBufferSize());
        return shader;
    }
} // namespace vzt
