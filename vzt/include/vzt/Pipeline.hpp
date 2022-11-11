#ifndef VZT_PIPELINE_HPP
#define VZT_PIPELINE_HPP

#include "vzt/Core/Math.hpp"
#include "vzt/Program.hpp"
#include "vzt/Synchronization.hpp"

namespace vzt
{
    // TODO: Move to render pass ?
    struct Viewport
    {
        Extent2D extent;
        struct Scissor
        {
            Vec2u start;
            Vec2u size;
        } scissor;
    };

    enum class PolygonMode
    {
        Fill  = VK_POLYGON_MODE_FILL,
        Line  = VK_POLYGON_MODE_LINE,
        Point = VK_POLYGON_MODE_POINT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(PolygonMode, VkPolygonMode)

    enum class CullMode : uint32_t
    {
        None         = VK_CULL_MODE_NONE,
        Front        = VK_CULL_MODE_FRONT_BIT,
        Back         = VK_CULL_MODE_BACK_BIT,
        FrontAndBack = VK_CULL_MODE_FRONT_AND_BACK
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(CullMode, VkCullModeFlags)

    enum class FrontFace : uint32_t
    {
        CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        Clockwise        = VK_FRONT_FACE_CLOCKWISE
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(FrontFace, VkFrontFace)

    struct Rasterization
    {
        bool        depthClamp    = false;
        bool        discardEnable = false;
        float       lineWidth     = 1.f;
        PolygonMode polygonMode   = PolygonMode::Fill;
        CullMode    cullMode      = CullMode::Back;
        FrontFace   frontFace     = FrontFace::Clockwise; // Only if cull mode is not None
    };

    struct MultiSample
    {
        bool     enable      = false;
        uint32_t sampleCount = 1u; // /!\ must be a power of two \in [0, 64]
    };

    class Pipeline
    {
      public:
        Pipeline() = default;
        ~Pipeline();

        void compile();

        void            setViewport(Viewport viewport);
        const Viewport& getViewport() const;

      private:
        VkPipeline       m_vkHandle       = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        Program m_program;

        Viewport      m_viewport;
        Rasterization m_rasterization;
        MultiSample   m_multiSample;
        // Optional<DescriptorLayout>       m_userDefinedDescriptorLayout;
        // Optional<VertexInputDescription> m_vertexInputDescription;
        // PipelineContextSettings          m_contextSettings{};
        // RasterizationOptions             m_rasterOptions{};
    };
} // namespace vzt

#include "vzt/Pipeline.inl"

#endif // VZT_PIPELINE_HPP
