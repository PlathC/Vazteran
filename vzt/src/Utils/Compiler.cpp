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
    }

    Compiler::~Compiler() = default;

    Compiler::Compiler(View<Instance> instance, const std::vector<vzt::Path>& includeDirectories) : m_instance(instance)
    {
        m_implementation = std::make_unique<Implementation>();

        // First we need to create slang global session with work with the Slang API.
        slang::createGlobalSession(m_implementation->globalSession.writeRef());

        // Next we create a compilation session to generate SPIRV code from Slang source.
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc  targetDesc  = {};
        targetDesc.format              = SLANG_SPIRV;
        targetDesc.profile             = m_implementation->globalSession->findProfile("spirv_1_5");
        targetDesc.flags               = 0;

        sessionDesc.targets     = &targetDesc;
        sessionDesc.targetCount = 1;

        std::vector<slang::CompilerOptionEntry> options;
        options.push_back({slang::CompilerOptionName::EmitSpirvDirectly,
                           {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}});
        sessionDesc.compilerOptionEntries    = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        m_implementation->globalSession->createSession(sessionDesc, m_implementation->session.writeRef());
    }

    Shader Compiler::operator()(const Path& path, const std::string& entryPoint) const
    {
        slang::IModule* module;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            module = m_implementation->session->loadModule(path.c_str(), diagnosticsBlob.writeRef());
            if (!module)
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnosticsBlob->getBufferPointer()));
                std::abort();
            }
        }

        int32_t entryPointCount = module->getDefinedEntryPointCount();

        bool foundEntryPoint = false;
        for (int32_t i = 0; i < entryPointCount; ++i)
        {
            Slang::ComPtr<slang::IEntryPoint> iEntryPoint;
            module->getDefinedEntryPoint(i, iEntryPoint.writeRef());
            printf("%s\n", iEntryPoint->getFunctionReflection()->getName());

            foundEntryPoint |= iEntryPoint->getFunctionReflection()->getName() == entryPoint;
        }
        VZT_ASSERT(foundEntryPoint);

        Slang::ComPtr<slang::IEntryPoint> iEntryPoint;
        module->findEntryPointByName(entryPoint.c_str(), iEntryPoint.writeRef());

        Slang::ComPtr<slang::IComponentType> composedProgram;
        {
            std::vector<slang::IComponentType*> componentTypes = {module, iEntryPoint};
            Slang::ComPtr<slang::IBlob>         diagnosticsBlob;
            if (SLANG_FAILED(m_implementation->session->createCompositeComponentType( //
                    componentTypes.data(), componentTypes.size(), composedProgram.writeRef(),
                    diagnosticsBlob.writeRef())))
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnosticsBlob->getBufferPointer()));
                std::abort();
            }
        }

        Slang::ComPtr<slang::IBlob> kernelBlob;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            if (SLANG_FAILED(
                    composedProgram->getEntryPointCode(0, 0, kernelBlob.writeRef(), diagnosticsBlob.writeRef())))
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnosticsBlob->getBufferPointer()));
                std::abort();
            }
        }
        printf("%s\n", (const char*)kernelBlob->getBufferPointer());

        slang::ProgramLayout*        programLayout = composedProgram->getLayout();
        slang::EntryPointReflection* reflection    = programLayout->findEntryPointByName(entryPoint.c_str());

        Shader shader = Shader(toShaderStage(reflection->getStage()), {});
        shader.compiledSource.resize(kernelBlob->getBufferSize() / sizeof(uint32_t));
        std::memcpy(shader.compiledSource.data(), kernelBlob->getBufferPointer(), kernelBlob->getBufferSize());
        return shader;
    }
} // namespace vzt
