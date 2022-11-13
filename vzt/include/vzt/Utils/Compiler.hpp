#ifndef VZT_UTILS_COMPILER_HPP
#define VZT_UTILS_COMPILER_HPP

#include <vector>

#include "vzt/Core/File.hpp"
#include "vzt/Vulkan/Program.hpp"

namespace vzt
{
    enum class ShadingLanguage
    {
        GLSL,
        HLSL
    };

    class Compiler
    {
      public:
        Compiler();

        Shader compile(const Path& path, ShaderStage stage, ShadingLanguage language, bool optimize) const;
        Shader compile(const Path& path, ShaderStage stage, ShadingLanguage language = ShadingLanguage::GLSL) const;

      private:
        static std::atomic<bool> IsInitialized;

        std::vector<Path> m_includePaths{};
    };
} // namespace vzt

#endif // VZT_UTILS_COMPILER_HPP
