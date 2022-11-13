#ifndef VZT_RENDERPASS_HPP
#define VZT_RENDERPASS_HPP

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"
#include "vzt/Vulkan/Image.hpp"
#include "vzt/Vulkan/Synchronization.hpp"

namespace vzt
{
    class Device;

    enum class AttachmentAccess : uint32_t
    {
        IndirectCommandRead               = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        IndexRead                         = VK_ACCESS_INDEX_READ_BIT,
        VertexAttributeRead               = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        UniformRead                       = VK_ACCESS_UNIFORM_READ_BIT,
        InputAttachmentRead               = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        ShaderRead                        = VK_ACCESS_SHADER_READ_BIT,
        ShaderWrite                       = VK_ACCESS_SHADER_WRITE_BIT,
        ColorAttachmentRead               = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        ColorAttachmentWrite              = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        DepthStencilAttachmentRead        = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        DepthStencilAttachmentWrite       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        TransferRead                      = VK_ACCESS_TRANSFER_READ_BIT,
        TransferWrite                     = VK_ACCESS_TRANSFER_WRITE_BIT,
        HostRead                          = VK_ACCESS_HOST_READ_BIT,
        HostWrite                         = VK_ACCESS_HOST_WRITE_BIT,
        MemoryRead                        = VK_ACCESS_MEMORY_READ_BIT,
        MemoryWrite                       = VK_ACCESS_MEMORY_WRITE_BIT,
        None                              = VK_ACCESS_NONE,
        TransformFeedbackWrite            = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
        TransformFeedbackCounterRead      = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
        TransformFeedbackCounterWrite     = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
        ConditionalRenderingRead          = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
        ColorAttachmentReadNonCoherent    = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
        AccelerationStructureRead         = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        AccelerationStructureWrite        = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        FragmentDensityMapRead            = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
        FragmentShadingRateAttachmentRead = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(AttachmentAccess)
    VZT_DEFINE_TO_VULKAN_FUNCTION(AttachmentAccess, VkAccessFlags)

    enum class PipelineBindPoint : uint32_t
    {
        Graphics      = VK_PIPELINE_BIND_POINT_GRAPHICS,
        Compute       = VK_PIPELINE_BIND_POINT_COMPUTE,
        RaytracingKHR = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(PipelineBindPoint, VkPipelineBindPoint)

    enum class DependencyFlag : uint32_t
    {
        None        = 0,
        ByRegion    = VK_DEPENDENCY_BY_REGION_BIT,
        DeviceGroup = VK_DEPENDENCY_DEVICE_GROUP_BIT,
        ViewLocal   = VK_DEPENDENCY_VIEW_LOCAL_BIT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(DependencyFlag, VkDependencyFlags)

    enum class LoadOp : uint32_t
    {
        Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
        Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
        DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(LoadOp, VkAttachmentLoadOp)

    enum class StoreOp : uint32_t
    {
        Store    = VK_ATTACHMENT_STORE_OP_STORE,
        DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        NoneQCOM = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
        NoneExt  = VK_ATTACHMENT_STORE_OP_NONE_EXT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(StoreOp, VkAttachmentStoreOp)

    struct AttachmentUse
    {
        Format      format;
        ImageLayout initialLayout;
        ImageLayout finalLayout;
        ImageLayout usedLayout;

        LoadOp      loadOp         = LoadOp::Clear;
        StoreOp     storeOp        = StoreOp::Store;
        LoadOp      stencilLoapOp  = LoadOp::DontCare;
        StoreOp     stencilStoreOp = StoreOp::DontCare;
        SampleCount sampleCount    = SampleCount::Sample1;
        Vec4        clearValue     = {0.f};
    };

    struct SubpassDependency
    {
        static constexpr uint32_t ExternalSubpass = ~0;

        uint32_t src;
        uint32_t dst;

        PipelineStage    srcStage;
        AttachmentAccess srcAccess;

        PipelineStage    dstStage;
        AttachmentAccess dstAccess;

        DependencyFlag dependencyFlags = DependencyFlag::ByRegion;
    };

    class RenderPass
    {
      public:
        RenderPass(View<Device> device);

        RenderPass(const RenderPass&)            = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        RenderPass(RenderPass&& other) noexcept;
        RenderPass& operator=(RenderPass&& other) noexcept;

        ~RenderPass();

        void addInput(AttachmentUse input);
        void addColor(AttachmentUse color);
        void setDepth(AttachmentUse depth);
        void addDependency(SubpassDependency dependency);
        void compile();

        inline VkRenderPass getHandle() const;

      private:
        View<Device> m_device{};
        VkRenderPass m_handle = VK_NULL_HANDLE;

        std::vector<AttachmentUse> m_inputAttachments{};
        std::vector<AttachmentUse> m_colorAttachments{};
        AttachmentUse              m_depthAttachment{};

        std::vector<SubpassDependency> m_dependencies{};
    };

} // namespace vzt

#include "vzt/Vulkan/RenderPass.inl"

#endif // VZT_RENDERPASS_HPP
