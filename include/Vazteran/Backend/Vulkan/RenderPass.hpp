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
		Transfer                     = VK_PIPELINE_STAGE_TRANSFER_BIT,
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
	BITWISE_FUNCTION(AttachmentAccess)
	TO_VULKAN_FUNCTION(AttachmentAccess, VkAccessFlagBits)

	enum class PipelineBindPoint : uint32_t
	{
		Graphics             = VK_PIPELINE_BIND_POINT_GRAPHICS,
		Compute              = VK_PIPELINE_BIND_POINT_COMPUTE,
		RaytracingKHR        = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		SubpassShadingHuawei = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI,
		RaytracingNV         = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
	};
	TO_VULKAN_FUNCTION(PipelineBindPoint, VkPipelineBindPoint)

	enum class DependencyFlag : uint32_t
	{
		ByRegion    = VK_DEPENDENCY_BY_REGION_BIT,
		DeviceGroup = VK_DEPENDENCY_DEVICE_GROUP_BIT,
		ViewLocal   = VK_DEPENDENCY_VIEW_LOCAL_BIT,
	};
	TO_VULKAN_FUNCTION(DependencyFlag, VkDependencyFlagBits)

	struct AttachmentPassUse
	{
		vzt::ImageLayout    initialLayout;
		vzt::ImageLayout    finalLayout;
		vzt::ImageLayout    usedLayout;
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
		vzt::ImageLayout    usedLayout;
		vzt::LoadOperation  loadOp;
		vzt::LoadOperation  stencilLoapOp;
		vzt::StoreOperation storeOp;
		vzt::StoreOperation stencilStoreOp;

		vzt::Vec2 clearValue = {1.f, 0.f};
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

	using AttachmentPassConfiguration      = std::pair<Attachment*, AttachmentPassUse>;
	using DepthAttachmentPassConfiguration = std::pair<Attachment*, DepthAttachmentPassUse>;
	struct RenderPassConfiguration
	{
		std::vector<AttachmentPassConfiguration> inputAttachments;
		std::vector<AttachmentPassConfiguration> colorAttachments;
		DepthAttachmentPassConfiguration         depthAttachment;

		std::vector<SubpassDependency> dependencies;
	};

	class RenderPass
	{
	  public:
	  public:
		RenderPass(const vzt::Device* device, const RenderPassConfiguration& configuration);

		RenderPass(const RenderPass&)            = delete;
		RenderPass& operator=(const RenderPass&) = delete;

		RenderPass(RenderPass&& other) noexcept;
		RenderPass& operator=(RenderPass&& other) noexcept;

		~RenderPass();

		void bind(VkCommandBuffer commandBuffer, const vzt::FrameBuffer* const frameBuffer) const;
		void unbind(VkCommandBuffer commandBuffer) const;

		VkRenderPass vkHandle() const { return m_vkHandle; }

	  private:
		VkRenderPass              m_vkHandle = VK_NULL_HANDLE;
		const vzt::Device*        m_device   = nullptr;
		std::vector<VkClearValue> m_clearValues;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERPASS_HPP
