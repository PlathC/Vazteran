#ifndef VZT_VULKAN_PROGRAM_HPP
#define VZT_VULKAN_PROGRAM_HPP

#include <string>
#include <vector>

#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/Core.hpp"
#include "vzt/Vulkan/Descriptor.hpp"

namespace vzt
{
    enum class ShaderStage : uint32_t
    {
        All                    = VK_SHADER_STAGE_ALL,
        AllGraphics            = VK_SHADER_STAGE_ALL_GRAPHICS,
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
    VZT_DEFINE_TO_VULKAN_FUNCTION(ShaderStage, VkShaderStageFlagBits);

    enum class ShaderGroupType : uint8_t
    {
        General            = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
        TrianglesHitGroup  = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
        ProceduralHitGroup = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ShaderGroupType, VkRayTracingShaderGroupTypeKHR);

    struct Shader
    {
        std::string                name;
        ShaderStage                stage;
        std::vector<uint32_t>      compiledSource;
        DescriptorLayout::Bindings bindings;

        struct hash
        {
            inline std::size_t operator()(const Shader& handle) const;
        };
    };

    class Device;
    class ShaderModule : public DeviceObject<VkShaderModule>
    {
      public:
        ShaderModule(View<Device> device, Shader shader);

        ShaderModule(const ShaderModule&)            = delete;
        ShaderModule& operator=(const ShaderModule&) = delete;

        ShaderModule(ShaderModule&& other) noexcept;
        ShaderModule& operator=(ShaderModule&& other) noexcept;

        ~ShaderModule() override;

        inline VkShaderModule getHandle() const;
        inline const Shader&  getShader() const;

      private:
        Shader m_shader = {};
    };

    class Program
    {
      public:
        Program() = default;
        Program(View<Device> device);
        Program(View<Device> device, std::vector<Shader> shaders);

        Program(const Program&)            = delete;
        Program& operator=(const Program&) = delete;

        Program(Program&& other) noexcept;
        Program& operator=(Program&& other) noexcept;

        ~Program() = default;

        inline void                             setShader(Shader shader);
        inline const std::vector<ShaderModule>& getModules() const;

      private:
        View<Device>              m_device        = {};
        std::vector<ShaderModule> m_shaderModules = {};
    };

    struct ShaderGroupShader
    {
        ShaderGroupType hitGroupType;
        ShaderModule    shaderModule;
    };

    class ShaderGroup
    {
      public:
        ShaderGroup(View<Device> device);

        ShaderGroup(const ShaderGroup&)            = delete;
        ShaderGroup& operator=(const ShaderGroup&) = delete;

        ShaderGroup(ShaderGroup&& other) noexcept;
        ShaderGroup& operator=(ShaderGroup&& other) noexcept;

        ~ShaderGroup() = default;

        void addShader(Shader shader, ShaderGroupType hitGroupType = ShaderGroupType::General);

        inline CSpan<ShaderGroupShader> getShaders() const;
        inline std::size_t              size() const;

      private:
        View<Device>                   m_device;
        std::vector<ShaderGroupShader> m_shaders;
    };

} // namespace vzt

#include "vzt/Vulkan/Program.inl"

#endif // VZT_PROGRAM_HPP
