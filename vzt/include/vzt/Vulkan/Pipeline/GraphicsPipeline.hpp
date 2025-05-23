#ifndef VZT_GRAPHIC_PIPELINE_HPP
#define VZT_GRAPHIC_PIPELINE_HPP

#include "vzt/Core/Math.hpp"
#include "vzt/Vulkan/Pipeline/Pipeline.hpp"
#include "vzt/Vulkan/Program.hpp"
#include "vzt/Vulkan/Synchronization.hpp"

namespace vzt
{
    class RenderPass;

    enum class ColorComponent
    {
        R    = VK_COLOR_COMPONENT_R_BIT,
        G    = VK_COLOR_COMPONENT_G_BIT,
        B    = VK_COLOR_COMPONENT_B_BIT,
        A    = VK_COLOR_COMPONENT_A_BIT,
        RGBA = R | G | B | A,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ColorComponent, VkColorComponentFlags)
    VZT_DEFINE_BITWISE_FUNCTIONS(ColorComponent)

    enum class CompareOp
    {
        Never          = VK_COMPARE_OP_NEVER,
        Less           = VK_COMPARE_OP_LESS,
        Equal          = VK_COMPARE_OP_EQUAL,
        LessOrEqual    = VK_COMPARE_OP_LESS_OR_EQUAL,
        Greater        = VK_COMPARE_OP_GREATER,
        NotEqual       = VK_COMPARE_OP_NOT_EQUAL,
        GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
        Always         = VK_COMPARE_OP_ALWAYS,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(CompareOp, VkCompareOp)
    using ColorMask = ColorComponent;

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

    enum class PolygonMode
    {
        Fill  = VK_POLYGON_MODE_FILL,
        Line  = VK_POLYGON_MODE_LINE,
        Point = VK_POLYGON_MODE_POINT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(PolygonMode, VkPolygonMode)

    enum class VertexInputRate
    {
        Vertex   = VK_VERTEX_INPUT_RATE_VERTEX,
        Instance = VK_VERTEX_INPUT_RATE_INSTANCE
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(VertexInputRate, VkVertexInputRate)

    enum class PrimitiveTopology
    {
        PointList                  = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        LineList                   = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        LineStrip                  = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
        TriangleList               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        TriangleStrip              = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        TriangleFan                = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
        LineListWithAdjacency      = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
        LineStripWithAdjacency     = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
        TriangleListWithAdjacency  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
        TriangleStripWithAdjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
        PatchList                  = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(PrimitiveTopology, VkPrimitiveTopology)

    enum class BlendOp
    {
        Add             = VK_BLEND_OP_ADD,
        Subtract        = VK_BLEND_OP_SUBTRACT,
        ReverseSubtract = VK_BLEND_OP_REVERSE_SUBTRACT,
        Min             = VK_BLEND_OP_MIN,
        Max             = VK_BLEND_OP_MAX,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(BlendOp, VkBlendOp)

    enum class BlendFactor
    {
        Zero                  = VK_BLEND_FACTOR_ZERO,
        One                   = VK_BLEND_FACTOR_ONE,
        SrcColor              = VK_BLEND_FACTOR_SRC_COLOR,
        OneMinusSrcColor      = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        DstColor              = VK_BLEND_FACTOR_DST_COLOR,
        OneMinusDstColor      = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        SrcAlpha              = VK_BLEND_FACTOR_SRC_ALPHA,
        OneMinusSrcAlpha      = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        DstAlpha              = VK_BLEND_FACTOR_DST_ALPHA,
        OneMinusDstAlpha      = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
        ConstantColor         = VK_BLEND_FACTOR_CONSTANT_COLOR,
        OneMinusConstantColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
        ConstantAlpha         = VK_BLEND_FACTOR_CONSTANT_ALPHA,
        OneMinusConstantAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
        SrcAlphaSaturate      = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
        Src1Color             = VK_BLEND_FACTOR_SRC1_COLOR,
        OneMinusSrc1Color     = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
        Src1Alpha             = VK_BLEND_FACTOR_SRC1_ALPHA,
        OneMinusSrc1Alpha     = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(BlendFactor, VkBlendFactor)

    struct Viewport
    {
        Extent2D size;
        Vec2u    upperLeftCorner = {0, 0};

        float minDepth = 0.f;
        float maxDepth = 1.f;

        struct Scissor
        {
            Extent2D size;
            Vec2i    offset = {0, 0};
        };
        Optional<Scissor> scissor{};
    };

    struct Rasterization
    {
        bool        depthClamp    = false;
        bool        discardEnable = false;
        float       lineWidth     = 1.f;
        PolygonMode polygonMode   = PolygonMode::Fill;
        CullMode    cullMode      = CullMode::Back;
        FrontFace   frontFace     = FrontFace::CounterClockwise; // Only if cull mode is not None
    };

    struct MultiSampling
    {
        bool        enable      = false;
        SampleCount sampleCount = SampleCount::Sample1;
    };

    struct DepthStencil
    {
        bool      enable           = true;
        bool      depthWriteEnable = true;
        CompareOp compareOp        = CompareOp::LessOrEqual;
    };

    struct VertexAttribute
    {
        uint32_t offset;
        uint32_t location;
        Format   dataFormat;
        uint32_t binding;
    };

    struct VertexBinding
    {
        uint32_t        binding;
        uint32_t        stride;
        VertexInputRate inputRate = VertexInputRate::Vertex;

        template <class Type>
        static VertexBinding   Typed(uint32_t binding, VertexInputRate inputRate = VertexInputRate::Vertex);
        inline VertexAttribute getAttribute(uint32_t location, Format dataFormat, uint32_t offset);
    };

    struct VertexInputDescription
    {
        std::vector<VertexBinding>   bindings;
        std::vector<VertexAttribute> attributes;

        inline void add(VertexBinding binding);
        inline void add(VertexAttribute binding);
        inline void add(uint32_t offset, uint32_t location, Format dataFormat, uint32_t binding);
    };

    struct ColorBlend
    {
        bool           blendEnable;
        BlendOp        colorBlendOp        = BlendOp::Add;
        BlendOp        alphaBlendOp        = BlendOp::Add;
        BlendFactor    srcColorBlendFactor = BlendFactor::SrcAlpha;
        BlendFactor    dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
        BlendFactor    srcAlphaBlendFactor = BlendFactor::One;
        BlendFactor    dstAlphaBlendFactor = BlendFactor::Zero;
        ColorComponent colorWriteMask      = ColorMask::RGBA;
    };

    class GraphicPipeline : public Pipeline
    {
      public:
        GraphicPipeline() = default;
        GraphicPipeline(const Program& program);

        GraphicPipeline(const GraphicPipeline&)            = delete;
        GraphicPipeline& operator=(const GraphicPipeline&) = delete;

        GraphicPipeline(GraphicPipeline&&) noexcept;
        GraphicPipeline& operator=(GraphicPipeline&&) noexcept;

        ~GraphicPipeline();

        void setProgram(const Program& program);

        inline void setVertexInputDescription(VertexInputDescription vertexDescription);
        inline void setColorBlend(uint32_t attachmentId, ColorBlend colorBlend);

        inline void            setViewport(Viewport config);
        inline const Viewport& getViewport() const;
        inline Viewport&       getViewport();

        inline void                 setRasterization(Rasterization config);
        inline const Rasterization& getRasterization() const;
        inline Rasterization&       getRasterization();

        inline void                 setMultiSampling(MultiSampling config);
        inline const MultiSampling& getMultiSampling() const;
        inline MultiSampling&       getMultiSampling();

        inline void                setDepthStencil(DepthStencil config);
        inline const DepthStencil& getDepthStencil() const;
        inline DepthStencil&       getDepthStencil();

        inline void                     setPrimitiveTopology(PrimitiveTopology topology);
        inline const PrimitiveTopology& getPrimitiveTopology() const;
        inline PrimitiveTopology&       getPrimitiveTopology();

        void compile(View<RenderPass> renderPass);
        void resize(Viewport viewport);

      private:
        void cleanup();

        View<Program> m_program;
        Viewport      m_viewport;

        std::unordered_map<uint32_t /* id */, ColorBlend> m_colorBlends;

        Optional<VertexInputDescription> m_vertexDescription = {};
        Rasterization                    m_rasterization;
        MultiSampling                    m_multiSample;
        DepthStencil                     m_depthStencil;
        PrimitiveTopology                m_primitiveTopology = PrimitiveTopology::TriangleList;

        View<RenderPass> m_cachedRenderPass;
        bool             m_compiled = false;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline/GraphicsPipeline.inl"

#endif // VZT_GRAPHIC_PIPELINE_HPP
