#ifndef VZT_VULKAN_PROGRAM_HPP
#define VZT_VULKAN_PROGRAM_HPP

#include <string>
#include <vector>

#include "vzt/core/type.hpp"
#include "vzt/vulkan/descriptor.hpp"
#include "vzt/vulkan/setup.hpp"

namespace vzt
{
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

#include "vzt/vulkan/program.inl"

#endif // VZT_PROGRAM_HPP
