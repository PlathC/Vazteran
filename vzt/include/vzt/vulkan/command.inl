#include "vzt/vulkan/command.hpp"

namespace vzt
{
    template <class Type>
    void CommandBuffer::pushConstants(const Pipeline& pipeline, ShaderStage stages, const Type& data)
    {
        pushConstants(pipeline, stages, 0, sizeof(Type), reinterpret_cast<const uint8_t*>(&data));
    }
} // namespace vzt