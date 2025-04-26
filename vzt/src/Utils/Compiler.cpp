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

    DescriptorType toDescriptorType(slang::VariableLayoutReflection* variable)
    {
        slang::TypeLayoutReflection* type = variable->getTypeLayout();
        VZT_ASSERT(variable->getCategory() == slang::ParameterCategory::DescriptorTableSlot);

        switch (type->getKind())
        {
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer: return DescriptorType::UniformBuffer;

        case slang::TypeReflection::Kind::SamplerState: return DescriptorType::CombinedSampler;
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
            case SLANG_TEXTURE_SUBPASS_MULTISAMPLE: return DescriptorType::CombinedSampler;

            case SLANG_TEXTURE_BUFFER: return DescriptorType::StorageTexelBuffer;

            case SLANG_RESOURCE_BASE_SHAPE_MASK:
            case SLANG_RESOURCE_UNKNOWN:
            case SLANG_RESOURCE_NONE:; VZT_NOT_REACHED();
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
        case slang::TypeReflection::Kind::Scalar: VZT_NOT_REACHED();
        }

        VZT_NOT_REACHED();
        return DescriptorType::None;
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

        const uint32_t parameterCount = programLayout->getParameterCount();
        for (uint32_t p = 0; p < parameterCount; p++)
        {
            slang::VariableLayoutReflection* variableLayout = programLayout->getParameterByIndex(p);

            const uint32_t bindingId   = variableLayout->getBindingIndex();
            shader.bindings[bindingId] = toDescriptorType(variableLayout);
        }

        return shader;
    }
} // namespace vzt
