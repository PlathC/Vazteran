#ifndef VZT_RENDERPASS_HPP
#define VZT_RENDERPASS_HPP

#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/image.hpp"

namespace vzt
{
    class Device;

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
        Vec4        clearValue     = vzt::Vec4{0.f};
    };

    struct SubpassDependency
    {
        static constexpr uint32_t ExternalSubpass = ~uint32_t(0);

        uint32_t src;
        uint32_t dst;

        PipelineStage    srcStage;
        AttachmentAccess srcAccess;

        PipelineStage    dstStage;
        AttachmentAccess dstAccess;

        DependencyFlag dependencyFlags = DependencyFlag::ByRegion;
    };

    class RenderPass : public DeviceObject<VkRenderPass>
    {
      public:
        RenderPass() = default;
        RenderPass(View<Device> device);

        RenderPass(const RenderPass&)            = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        RenderPass(RenderPass&& other) noexcept;
        RenderPass& operator=(RenderPass&& other) noexcept;

        ~RenderPass() override;

        void addInput(AttachmentUse input);
        void addColor(AttachmentUse color);
        void setDepth(AttachmentUse depth);
        void addDependency(SubpassDependency dependency);
        void compile();

        inline const std::vector<AttachmentUse>& getInputAttachments() const;
        inline const std::vector<AttachmentUse>& getColorAttachments() const;
        inline const AttachmentUse&              getDepthAttachment() const;

      private:
        std::vector<AttachmentUse> m_inputAttachments{};
        std::vector<AttachmentUse> m_colorAttachments{};
        AttachmentUse              m_depthAttachment{};

        std::vector<SubpassDependency> m_dependencies{};
    };

} // namespace vzt

#include "vzt/vulkan/render_pass.inl"

#endif // VZT_RENDERPASS_HPP
