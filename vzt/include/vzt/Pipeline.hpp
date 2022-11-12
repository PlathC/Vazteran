#ifndef VZT_PIPELINE_HPP
#define VZT_PIPELINE_HPP

#include <optional>

#include "vzt/Core/Math.hpp"
#include "vzt/Program.hpp"
#include "vzt/Synchronization.hpp"

namespace vzt
{
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
        std::optional<Scissor> scissor{};
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

    struct MultiSampling
    {
        bool     enable      = false;
        uint32_t sampleCount = 1u; // /!\ must be a power of two \in [0, 64]
    };

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

    struct DepthStencil
    {
        bool      enable           = true;
        bool      depthWriteEnable = true;
        CompareOp compareOp        = CompareOp::LessOrEqual;
    };

    class Pipeline
    {
      public:
        Pipeline(View<Device> device);

        Pipeline(const Pipeline&);
        Pipeline& operator=(const Pipeline&);

        Pipeline(Pipeline&&) noexcept;
        Pipeline& operator=(Pipeline&&) noexcept;

        ~Pipeline();

        void compile();

        inline void            setViewport(Viewport viewport);
        inline const Viewport& getViewport() const;
        inline Viewport&       getViewport();

        inline void                 setRasterization(Rasterization viewport);
        inline const Rasterization& getRasterization() const;
        inline Rasterization&       getRasterization();

        inline void                 setMultiSampling(MultiSampling viewport);
        inline const MultiSampling& getMultiSampling() const;
        inline MultiSampling&       getMultiSampling();

        inline void                setDepthStencil(DepthStencil viewport);
        inline const DepthStencil& getDepthStencil() const;
        inline DepthStencil&       getDepthStencil();

      private:
        View<Device>     m_device;
        VkPipeline       m_handle         = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        Viewport      m_viewport = {{0u, 0u}};
        Rasterization m_rasterization;
        MultiSampling m_multiSample;
        DepthStencil  m_depthStencil;
    };
} // namespace vzt

#include "vzt/Pipeline.inl"

#endif // VZT_PIPELINE_HPP
