#ifndef VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP

#include "GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/Attachment.hpp"

namespace vzt
{
	class Device;
	class FrameBuffer;

	enum class PipelineStage : uint32_t
	{
		TopOfPipe                    = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		DrawIndirect                 = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		VertexInput                  = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		VertexShader                 = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		TessellationControlShader    = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
		TessellationEvaluationShader = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
		GeometryShader               = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
		FragmentShader               = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		EarlyFragmentTests           = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		LateFragmentTests            = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		ColorAttachmentOutput        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		ComputeShader                = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		StageTransfer                = VK_PIPELINE_STAGE_TRANSFER_BIT,
		BottomOfPipe                 = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		Host                         = VK_PIPELINE_STAGE_HOST_BIT,
		AllGraphic                   = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		AllCommands                  = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		TransformFeedback            = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
		ConditionRendering           = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
		AccelerationStructureBuild   = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		RaytracingShader             = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		TaskShaderNV                 = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
		MeshShaderNV                 = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
		None                         = VK_PIPELINE_STAGE_NONE_KHR,
	};
	BITWISE_FUNCTION(PipelineStage)
	TO_VULKAN_FUNCTION(PipelineStage, VkPipelineStageFlagBits)

	enum class PipelineBindPoint : uint32_t
	{
		Graphics             = VK_PIPELINE_BIND_POINT_GRAPHICS,
		Compute              = VK_PIPELINE_BIND_POINT_COMPUTE,
		RaytracingKHR        = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		SubpassShadingHuawei = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI,
		RaytracingNV         = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
	};
	TO_VULKAN_FUNCTION(PipelineBindPoint, VkPipelineBindPoint)

	struct AttachmentPassUse
	{
		vzt::ImageLayout    initialLayout;
		vzt::ImageLayout    finalLayout;
		vzt::LoadOperation  loadOp;
		vzt::LoadOperation  stencilLoapOp;
		vzt::StoreOperation storeOp;
		vzt::StoreOperation stencilStoreOp;

		vzt::Vec4 clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
	};

	struct DepthAttachmentPassUse
	{
		vzt::ImageLayout    initialLayout;
		vzt::ImageLayout    finalLayout;
		vzt::LoadOperation  loadOp;
		vzt::LoadOperation  stencilLoapOp;
		vzt::StoreOperation storeOp;
		vzt::StoreOperation stencilStoreOp;

		vzt::Vec2 clearValue = {1.f, 0.f};
	};

	class RenderPass
	{
	  public:
		using AttachmentPassConfiguration      = std::pair<vzt::Attachment*, vzt::AttachmentPassUse>;
		using DepthAttachmentPassConfiguration = std::pair<vzt::Attachment*, vzt::DepthAttachmentPassUse>;

	  public:
		RenderPass(const vzt::Device* device, const std::vector<AttachmentPassConfiguration>& attachments,
		           const DepthAttachmentPassConfiguration& depthAttachment);

		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;

		RenderPass(RenderPass&& other) noexcept;
		RenderPass& operator=(RenderPass&& other) noexcept;

		~RenderPass();

		void bind(VkCommandBuffer commandBuffer, const vzt::FrameBuffer* const frameBuffer) const;
		void unbind(VkCommandBuffer commandBuffer) const;

		VkRenderPass vkHandle() const { return m_vkHandle; }

	  private:
		VkRenderPass       m_vkHandle = VK_NULL_HANDLE;
		const vzt::Device* m_device   = nullptr;

		std::vector<VkSubpassDependency> m_subpassDependencies;

		std::vector<VkAttachmentReference> m_colorRefs;
		VkAttachmentReference              m_depthRef{};
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP
