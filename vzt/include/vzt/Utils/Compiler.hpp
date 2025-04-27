#ifndef VZT_UTILS_COMPILER_HPP
#define VZT_UTILS_COMPILER_HPP

#include <vector>

#include "vzt/Core/File.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    class Instance;

    class Compiler
    {
      public:
        Compiler() = default;
        Compiler(View<Instance> instance, const std::vector<vzt::Path>& includeDirectories = {"."});
        ~Compiler();

        Shader              operator()(const Path& path, const std::string& entryPoint) const;
        std::vector<Shader> operator()(const Path& path) const;

      private:
        View<Instance>    m_instance{};
        std::vector<Path> m_includePaths{};

        struct Implementation;
        std::unique_ptr<Implementation> m_implementation;
    };
} // namespace vzt

#endif // VZT_UTILS_COMPILER_HPP
