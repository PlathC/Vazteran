#include "vzt/vulkan/pipeline/graphics.hpp"

namespace vzt
{
    template <class Type>
    VertexBinding VertexBinding::Typed(uint32_t binding, VertexInputRate inputRate)
    {
        return {binding, sizeof(Type), inputRate};
    }

    inline VertexAttribute VertexBinding::getAttribute(uint32_t location, Format dataFormat, uint32_t offset)
    {
        return {offset, location, dataFormat, binding};
    }

    inline void VertexInputDescription::add(VertexBinding binding) { bindings.emplace_back(std::move(binding)); }
    inline void VertexInputDescription::add(VertexAttribute binding) { attributes.emplace_back(std::move(binding)); }
    inline void VertexInputDescription::add(uint32_t offset, uint32_t location, Format dataFormat, uint32_t binding)
    {
        add(VertexAttribute{offset, location, dataFormat, binding});
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColor(Format format, ColorBlend blend)
    {
        colors.emplace_back(format, blend);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDepth(Format format)
    {
        depth = std::move(format);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set(VertexInputDescription desc)
    {
        inputDescription = std::move(desc);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set(Rasterization rast)
    {
        rasterization = std::move(rast);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set(MultiSampling multi)
    {
        multiSampling = std::move(multi);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set(DepthStencil depthSt)
    {
        depthStencil = std::move(depthSt);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set(PrimitiveTopology prim)
    {
        primitiveTopology = std::move(prim);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::add(PushConstant constant)
    {
        pushConstants.push_back(std::move(constant));
        return *this;
    }
} // namespace vzt
