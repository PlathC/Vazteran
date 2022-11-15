#ifndef VZT_PIPELINE_HPP
#define VZT_PIPELINE_HPP

#include "vzt/Core/Math.hpp"
#include "vzt/Vulkan/Descriptor.hpp"
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
        FrontFace   frontFace     = FrontFace::Clockwise; // Only if cull mode is not None
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
        static VertexBinding Typed(uint32_t binding, VertexInputRate inputRate);

        inline VertexAttribute getAttribute(uint32_t location, Format dataFormat, uint32_t offset);
    };

    struct VertexInputDescription
    {
        std::vector<VertexBinding>   bindings;
        std::vector<VertexAttribute> attributes;

        inline void add(VertexBinding binding);
        inline void add(VertexAttribute binding);
    };

    class Pipeline
    {
      public:
        Pipeline() = default;
        Pipeline(View<Device> device, View<Program> program, Viewport viewport);

        Pipeline(const Pipeline&)            = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        Pipeline(Pipeline&&) noexcept;
        Pipeline& operator=(Pipeline&&) noexcept;

        ~Pipeline();

        inline void setVertexInputDescription(VertexInputDescription vertexDescription);
        inline void addAttachmentColorBlend(ColorMask mask = ColorComponent::RGBA);
        inline void addAttachmentsColorBlend(std::size_t nb, ColorMask mask = ColorComponent::RGBA);
        inline void setProgram(const Program& program);
        inline void setDescriptorLayout(DescriptorLayout descriptorLayout);

        inline void            setTargetSize(Extent2D targetSize);
        inline const Extent2D& getTargetSize() const;
        inline Extent2D&       getTargetSize();

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

        void compile(View<RenderPass> renderPass);

      private:
        void cleanup();

        View<Device>     m_device;
        VkPipeline       m_handle         = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        View<Program>    m_program;
        Viewport         m_viewport;

        std::vector<ColorMask> m_attachments;
        DescriptorLayout       m_descriptorLayout;

        Optional<VertexInputDescription> m_vertexDescription = {};
        Rasterization                    m_rasterization;
        MultiSampling                    m_multiSample;
        DepthStencil                     m_depthStencil;

        bool m_compiled = false;
    };
} // namespace vzt

#include "vzt/Vulkan/Pipeline.inl"

#endif // VZT_PIPELINE_HPP
