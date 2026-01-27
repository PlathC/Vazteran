#include "vzt/vulkan/command.hpp"

namespace vzt
{
    template <class Type>
    void CommandBuffer::pushConstants(const Pipeline& pipeline, ShaderStage stages, const Type* data)
    {
        pushConstants(pipeline, stages, 0, sizeof(Type), data);
    }
} // namespace vzt