#include "vzt/compiler.hpp"

#include <unordered_map>

//
#include "vzt/core/disable_warnings.hpp"
//
#include <slang/slang.h>

#include <slang/slang-com-ptr.h>
//
#include "vzt/core/enable_warnings.hpp"
//

#include "vzt/core/assert.hpp"
#include "vzt/core/logger.hpp"
#include "vzt/vulkan/instance.hpp"

namespace vzt
{
    struct Compiler::Implementation
    {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        Slang::ComPtr<slang::ISession>       session;
    };

    struct Module::Implementation
    {
        slang::IModule* data;
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

    DescriptorType toDescriptorType(slang::VariableLayoutReflection* variable)
    {
        slang::TypeLayoutReflection* type = variable->getTypeLayout();
        VZT_ASSERT(variable->getCategory() == slang::ParameterCategory::DescriptorTableSlot);

        switch (type->getKind())
        {
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer: return DescriptorType::UniformBuffer;

        // TODO: Differentiate Sampler and CombinedSampler
        // case slang::TypeReflection::Kind::SamplerState: return DescriptorType::Sampler;
        case slang::TypeReflection::Kind::TextureBuffer: return DescriptorType::StorageTexelBuffer;
        case slang::TypeReflection::Kind::ShaderStorageBuffer: return DescriptorType::StorageBuffer;

        case slang::TypeReflection::Kind::Resource: {
            switch (type->getResourceShape())
            {
            case SLANG_BYTE_ADDRESS_BUFFER:;
            case SLANG_STRUCTURED_BUFFER: return DescriptorType::StorageBuffer;

            case SLANG_ACCELERATION_STRUCTURE: return DescriptorType::AccelerationStructure;

            case SLANG_TEXTURE_1D:
            case SLANG_TEXTURE_2D:
            case SLANG_TEXTURE_3D:
            case SLANG_TEXTURE_CUBE:
            case SLANG_TEXTURE_SUBPASS:;
            case SLANG_RESOURCE_EXT_SHAPE_MASK:;
            case SLANG_TEXTURE_FEEDBACK_FLAG:;
            case SLANG_TEXTURE_SHADOW_FLAG:;
            case SLANG_TEXTURE_ARRAY_FLAG:;
            case SLANG_TEXTURE_MULTISAMPLE_FLAG:;
            case SLANG_TEXTURE_1D_ARRAY:;
            case SLANG_TEXTURE_2D_ARRAY:;
            case SLANG_TEXTURE_CUBE_ARRAY:;
            case SLANG_TEXTURE_2D_MULTISAMPLE:;
            case SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY:;
            case SLANG_TEXTURE_SUBPASS_MULTISAMPLE:
                if (type->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ_WRITE)
                    return DescriptorType::StorageImage;
                return DescriptorType::CombinedSampler;

            case SLANG_TEXTURE_BUFFER: return DescriptorType::StorageTexelBuffer;

            case SLANG_RESOURCE_BASE_SHAPE_MASK:
            case SLANG_RESOURCE_UNKNOWN:
            case SLANG_RESOURCE_NONE:; return DescriptorType::None;
            }
        }

        case slang::TypeReflection::Kind::GenericTypeParameter: // ?
        case slang::TypeReflection::Kind::Interface:            // ?
        case slang::TypeReflection::Kind::OutputStream:         // ?
        case slang::TypeReflection::Kind::Specialized:          // ?
        case slang::TypeReflection::Kind::Feedback:             // ?
        case slang::TypeReflection::Kind::DynamicResource:      // ?

        case slang::TypeReflection::Kind::Pointer:;
        case slang::TypeReflection::Kind::None:
        case slang::TypeReflection::Kind::Struct:
        case slang::TypeReflection::Kind::Array:
        case slang::TypeReflection::Kind::Matrix:
        case slang::TypeReflection::Kind::Vector:
        case slang::TypeReflection::Kind::Scalar:
        default: return DescriptorType::None;
        }

        return DescriptorType::None;
    }

    Compiler::Compiler()                            = default;
    Compiler::Compiler(Compiler&& other)            = default;
    Compiler& Compiler::operator=(Compiler&& other) = default;
    Compiler::~Compiler()                           = default;

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

        std::vector<std::string> searchPathsStrs;
        std::vector<const char*> searchPaths;
        std::transform(begin(includeDirectories), end(includeDirectories), std::back_inserter(searchPathsStrs),
                       [](const vzt::Path& path) { return path.string(); });
        std::transform(begin(searchPathsStrs), end(searchPathsStrs), std::back_inserter(searchPaths),
                       [](const std::string& path) { return path.c_str(); });

        sessionDesc.searchPaths     = searchPaths.data();
        sessionDesc.searchPathCount = static_cast<SlangInt>(searchPaths.size());

        m_implementation->globalSession->createSession(sessionDesc, m_implementation->session.writeRef());
    }

    Shader Compiler::operator()(const Path& path, const std::string& entryPoint, CSpan<Module> modules) const
    {
        const std::string pathStr = path.string();

        Slang::ComPtr<slang::IBlob> diagnostics;
        slang::IModule*             module;
        {
            module = m_implementation->session->loadModule(pathStr.c_str(), diagnostics.writeRef());
            if (!module)
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
                std::abort();
            }
        }

        const int32_t entryPointCount = module->getDefinedEntryPointCount();

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
            for (uint32_t m = 0; m < modules.size; ++m)
                componentTypes.emplace_back(modules[m].implementation->data);

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

        Shader shader = Shader(entryPoint, toShaderStage(reflection->getStage()), {});
        shader.compiledSource.resize(kernelBlob->getBufferSize() / sizeof(uint32_t));
        std::memcpy(shader.compiledSource.data(), kernelBlob->getBufferPointer(), kernelBlob->getBufferSize());

        const uint32_t parameterCount = programLayout->getParameterCount();
        for (uint32_t p = 0; p < parameterCount; p++)
        {
            slang::VariableLayoutReflection* variableLayout = programLayout->getParameterByIndex(p);

            const uint32_t bindingId = variableLayout->getBindingIndex();

            const DescriptorType type = toDescriptorType(variableLayout);
            if (type == DescriptorType::None)
                continue;

            shader.bindings[bindingId] = type;
        }

        return shader;
    }

    std::vector<Shader> Compiler::operator()(const Path& path, CSpan<Module> modules) const
    {
        const std::string pathStr = path.string();

        Slang::ComPtr<slang::IBlob> diagnostics;
        slang::IModule*             module;
        {
            module = m_implementation->session->loadModule(pathStr.c_str(), diagnostics.writeRef());
            if (!module)
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
                std::abort();
            }
        }

        const int32_t       entryPointCount = module->getDefinedEntryPointCount();
        std::vector<Shader> shaders         = {};
        shaders.reserve(static_cast<std::size_t>(entryPointCount));

        for (int32_t i = 0; i < entryPointCount; ++i)
        {
            Slang::ComPtr<slang::IEntryPoint> iEntryPoint;
            module->getDefinedEntryPoint(i, iEntryPoint.writeRef());

            Slang::ComPtr<slang::IComponentType> composedProgram;
            {
                std::vector<slang::IComponentType*> componentTypes = {module, iEntryPoint};
                for (uint32_t m = 0; m < modules.size; ++m)
                    componentTypes.emplace_back(modules[m].implementation->data);

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
                if (SLANG_FAILED(
                        composedProgram->getEntryPointCode(0, 0, kernelBlob.writeRef(), diagnostics.writeRef())))
                {
                    vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                       reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
                    std::abort();
                }
            }

            const std::string entryPoint = iEntryPoint->getFunctionReflection()->getName();

            slang::ProgramLayout*        programLayout = composedProgram->getLayout();
            slang::EntryPointReflection* reflection    = programLayout->findEntryPointByName(entryPoint.c_str());

            Shader shader = Shader(entryPoint, toShaderStage(reflection->getStage()), {});
            shader.compiledSource.resize(kernelBlob->getBufferSize() / sizeof(uint32_t));
            std::memcpy(shader.compiledSource.data(), kernelBlob->getBufferPointer(), kernelBlob->getBufferSize());

            const uint32_t parameterCount = programLayout->getParameterCount();
            for (uint32_t p = 0; p < parameterCount; p++)
            {
                slang::VariableLayoutReflection* variableLayout = programLayout->getParameterByIndex(p);

                const uint32_t       bindingId = variableLayout->getBindingIndex();
                const DescriptorType type      = toDescriptorType(variableLayout);
                if (type == DescriptorType::None)
                    continue;

                shader.bindings[bindingId] = type;
            }

            shaders.emplace_back(std::move(shader));
        }

        return shaders;
    }

    Module::Module()                                   = default;
    Module::Module(Module&& other) noexcept            = default;
    Module& Module::operator=(Module&& other) noexcept = default;

    Module::~Module() = default;

    Module Compiler::load(const Path& path) const
    {
        const std::string pathStr = path.string();

        Slang::ComPtr<slang::IBlob> diagnostics;
        slang::IModule*             module;
        {
            module = m_implementation->session->loadModule(pathStr.c_str(), diagnostics.writeRef());
            if (!module)
            {
                vzt::logger::error("[SLANG] Compile Error, diagnostic {}",
                                   reinterpret_cast<const char*>(diagnostics->getBufferPointer()));
                std::abort();
            }
        }

        Module result         = {};
        result.implementation = std::make_unique<Module::Implementation>(module);

        return result;
    }
} // namespace vzt
