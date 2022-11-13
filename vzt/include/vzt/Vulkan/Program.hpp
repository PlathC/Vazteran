#ifndef VZT_VULKAN_PROGRAM_HPP
#define VZT_VULKAN_PROGRAM_HPP

#include <cstdint>
#include <unordered_set>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "vzt/Core/Type.hpp"

namespace vzt
{
    enum class ShaderStage : uint16_t
    {
        Vertex                 = VK_SHADER_STAGE_VERTEX_BIT,
        TessellationControl    = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        TessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        Geometry               = VK_SHADER_STAGE_GEOMETRY_BIT,
        Fragment               = VK_SHADER_STAGE_FRAGMENT_BIT,
        Compute                = VK_SHADER_STAGE_COMPUTE_BIT,
        RayGen                 = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        AnyHit                 = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        ClosestHit             = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        Miss                   = VK_SHADER_STAGE_MISS_BIT_KHR,
        Intersection           = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        Callable               = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        Task                   = VK_SHADER_STAGE_TASK_BIT_NV,
        Mesh                   = VK_SHADER_STAGE_MESH_BIT_NV
    };

    struct DescriptorSet
    {
        uint32_t binding;
    };

    struct SamplerDescriptorSet : public DescriptorSet
    {
    };

    struct SizedDescriptorSet : public DescriptorSet
    {
        uint32_t size;
    };

    struct Shader
    {
        ShaderStage           stage;
        std::vector<uint32_t> compiledSource;

        struct hash
        {
            inline std::size_t operator()(const Shader& handle) const;
        };
    };

    class Device;
    class ShaderModule
    {
      public:
        ShaderModule(View<Device> device, Shader shader);

        ShaderModule(const ShaderModule&)            = delete;
        ShaderModule& operator=(const ShaderModule&) = delete;

        ShaderModule(ShaderModule&& other) noexcept;
        ShaderModule& operator=(ShaderModule&& other) noexcept;

        ~ShaderModule();

        inline VkShaderModule getHandle() const;

      private:
        View<Device>   m_device = {};
        VkShaderModule m_handle = VK_NULL_HANDLE;
        Shader         shader   = {};
    };

    class Program
    {
      public:
        Program(View<Device> device);

        Program(const Program&)            = delete;
        Program& operator=(const Program&) = delete;

        Program(Program&& other) noexcept;
        Program& operator=(Program&& other) noexcept;

        ~Program() = default;

        void setShader(Shader shader);

      private:
        using ShaderList = std::unordered_set<Shader, Shader::hash>;

        View<Device>              m_device        = {};
        std::vector<ShaderModule> m_shaderModules = {};
    };
} // namespace vzt

#include "vzt/Vulkan/Program.inl"

#endif // VZT_PROGRAM_HPP
