#ifndef VZT_UTILS_COMPILER_HPP
#define VZT_UTILS_COMPILER_HPP

#include <vector>

#include "vzt/Core/File.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    class Instance;
    class Module;
    class Compiler
    {
      public:
        Compiler();
        Compiler(View<Instance> instance, const std::vector<vzt::Path>& includeDirectories = {"."});

        Compiler(Compiler&& other);
        Compiler& operator=(Compiler&& other);

        ~Compiler();

        Shader operator()(const Path& path, const std::string& entryPoint, CSpan<Module> modules = {}) const;
        std::vector<Shader> operator()(const Path& path, CSpan<Module> modules = {}) const;

        Module load(const Path& path) const;

      private:
        View<Instance>    m_instance{};
        std::vector<Path> m_includePaths{};

        struct Implementation;
        std::unique_ptr<Implementation> m_implementation;
    };

    class Module
    {
      public:
        friend Compiler;

        Module(Module&& other) noexcept;
        Module& operator=(Module&& other) noexcept;
        ~Module();

      private:
        Module();

        struct Implementation;
        std::unique_ptr<Implementation> implementation;
    };
} // namespace vzt

#endif // VZT_UTILS_COMPILER_HPP
