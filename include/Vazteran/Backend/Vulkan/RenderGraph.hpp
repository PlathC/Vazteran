#ifndef VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP

#include <functional>
#include <optional>

#include <vulkan/vulkan.hpp>

#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"

namespace vzt
{
	// References:
	// Hans-Kristian Arntzen, Render graphs and Vulkan — a deep dive,
	// https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

	enum class QueueType : uint32_t
	{
		Graphic = VK_QUEUE_GRAPHICS_BIT,
		Compute = VK_QUEUE_COMPUTE_BIT
	};
	TO_VULKAN_FUNCTION(QueueType, VkQueueFlagBits)

	enum class LoadOperation : uint32_t
	{
		Load     = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD,
		Clear    = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
		DontCare = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE
	};
	TO_VULKAN_FUNCTION(LoadOperation, VkAttachmentLoadOp)

	enum class StoreOperation : uint32_t
	{
		Store    = VK_ATTACHMENT_STORE_OP_STORE,
		DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		// NoneKHR  = VK_ATTACHMENT_STORE_OP_NONE_KHR,
		NoneQCOM = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
		NoneExt  = VK_ATTACHMENT_STORE_OP_NONE_EXT
	};
	TO_VULKAN_FUNCTION(StoreOperation, VkAttachmentStoreOp)

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

	enum class AccessFlag : uint32_t
	{
		IndirectCommandRead         = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
		IndexRead                   = VK_ACCESS_INDEX_READ_BIT,
		VertexAttributeRead         = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		UniformRead                 = VK_ACCESS_UNIFORM_READ_BIT,
		InputAttachmentRead         = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
		ShaderRead                  = VK_ACCESS_SHADER_READ_BIT,
		ShaderWrite                 = VK_ACCESS_SHADER_WRITE_BIT,
		ColorAttachmentRead         = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
		ColorAttachmentWrite        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		DepthStencilAttachmentRead  = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		DepthStencilAttachmentWrite = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		TransferRead                = VK_ACCESS_TRANSFER_READ_BIT,
		TransferWrite               = VK_ACCESS_TRANSFER_WRITE_BIT,
		HostRead                    = VK_ACCESS_HOST_READ_BIT,
		HostWrite                   = VK_ACCESS_HOST_WRITE_BIT,
		MemoryRead                  = VK_ACCESS_MEMORY_READ_BIT,
		MemoryWrite                 = VK_ACCESS_MEMORY_WRITE_BIT,
	};
	BITWISE_FUNCTION(AccessFlag)
	TO_VULKAN_FUNCTION(AccessFlag, VkAccessFlagBits)

	struct AttachmentSettings
	{
		std::optional<vzt::Format>        format{};    // if unset, use swapchain image formats
		std::optional<vzt::Size2D<float>> imageSize{}; // if unset, use frame buffer size
		vzt::SampleCount                  sampleCount = vzt::SampleCount::Sample1;
	};

	struct StorageSettings
	{
		std::size_t      size;
		vzt::BufferUsage usage;
	};

	struct AttachmentHandle
	{
		std::size_t id;        // immutable identifier
		std::size_t state = 0; // Describe the state of the handle

		struct hash
		{
			auto operator()(const AttachmentHandle& handle) const { return handle.id; }
		};

		bool operator==(const AttachmentHandle& other) const { return id == other.id && state == other.state; }
	};

	struct StorageHandle
	{
		std::size_t id;        // immutable identifier
		std::size_t state = 0; // Describe the state of the handle

		struct hash
		{
			auto operator()(const StorageHandle& handle) const { return handle.id; }
		};

		bool operator==(const StorageHandle& other) const { return id == other.id && state == other.state; }
	};

	struct AttachmentPassUse
	{
		vzt::ImageLayout    initialLayout;
		vzt::ImageLayout    finalLayout;
		vzt::LoadOperation  loadOp;
		vzt::LoadOperation  stencilLoapOp;
		vzt::StoreOperation storeOp;
		vzt::StoreOperation stencilStoreOp;
	};

	struct ImageBarrier
	{
		vzt::PipelineStage stages;
		vzt::AccessFlag    accesses;
		vzt::ImageLayout   layout;
	};

	struct StorageBarrier
	{
		vzt::PipelineStage stages;
		vzt::AccessFlag    accesses;
	};

	template <class Type>
	using AttachmentList = std::unordered_map<vzt::AttachmentHandle, Type, vzt::AttachmentHandle::hash>;
	template <class Type>
	using StorageList = std::unordered_map<vzt::StorageHandle, Type, vzt::StorageHandle::hash>;

	using RenderFunction     = std::function<void(VkCommandBuffer& /*cmd*/)>;
	using DepthClearFunction = std::function<bool(VkClearDepthStencilValue* /* value */)>;
	using ColorClearFunction = std::function<bool(uint32_t /* renderTargetIdx */, VkClearColorValue* /* value */)>;
	class RenderPassHandler
	{
	  public:
		friend class RenderGraph;

	  public:
		RenderPassHandler() = delete;

		void addColorInput(const vzt::AttachmentHandle attachment, const std::string& attachmentName = "");
		void addColorOutput(vzt::AttachmentHandle& attachment, const std::string& attachmentName = "");
		void addColorInputOutput(vzt::AttachmentHandle& attachment, const std::string& inName = "",
		                         const std::string& outName = "");

		void addStorageInput(const vzt::StorageHandle storage, const std::string& storageName = "");
		void addStorageOutput(vzt::StorageHandle& storage, const std::string& storageName = "");
		void addStorageInputOutput(vzt::StorageHandle& storage, const std::string& inName = "",
		                           const std::string& outName = "");

		void setDepthStencilInput(const vzt::AttachmentHandle depthStencil, const std::string& attachmentName = "");
		void setDepthStencilOutput(vzt::AttachmentHandle& depthStencil, const std::string& attachmentName = "");

		void setRenderFunction(vzt::RenderFunction renderFunction);
		void setDepthClearFunction(vzt::DepthClearFunction depthClearFunction);
		void setColorClearFunction(vzt::ColorClearFunction colorClearFunction);

		bool isDependingOn(const RenderPassHandler& other) const;

	  private:
		RenderPassHandler(const vzt::RenderGraph* const graph, std::string name, vzt::QueueType queueType);

	  private:
		const vzt::RenderGraph* m_graph;
		std::string             m_name;
		vzt::QueueType          m_queueType;

		struct AttachmentInfo
		{
			std::string       name;
			vzt::ImageBarrier barrier;
			AttachmentPassUse attachmentUse;
		};

		struct StorageInfo
		{
			std::string         name;
			vzt::StorageBarrier barrier;
		};

		vzt::AttachmentList<AttachmentInfo>                             m_colorInputs;
		vzt::StorageList<StorageInfo>                                   m_storageInputs;
		std::optional<std::pair<vzt::AttachmentHandle, AttachmentInfo>> m_depthInput;

		vzt::AttachmentList<AttachmentInfo>                             m_colorOutputs;
		vzt::StorageList<StorageInfo>                                   m_storageOutputs;
		std::optional<std::pair<vzt::AttachmentHandle, AttachmentInfo>> m_depthOutput;

		vzt::RenderFunction     m_renderFunction;
		vzt::DepthClearFunction m_depthClearFunction;
		vzt::ColorClearFunction m_colorClearFunction;
	};

	struct PhysicalAttachment
	{
		const vzt::AttachmentSettings settings;
	};

	struct PhysicalStorage
	{
		const vzt::StorageSettings settings;
		vzt::Buffer                buffer;
	};

	class RenderGraph
	{
	  public:
		RenderGraph();
		~RenderGraph();

		// User configuration
		vzt::AttachmentHandle addAttachment(const vzt::AttachmentSettings& settings);
		vzt::StorageHandle    addStorage(const vzt::StorageSettings& settings);

		vzt::RenderPassHandler& addPass(const std::string& name, const vzt::QueueType queueType);
		void                    setBackBuffer(const vzt::AttachmentHandle backBufferHandle);

		bool isBackBuffer(const vzt::AttachmentHandle backBufferHandle) const;

		// User information check
		void compile(vzt::Format scColorFormat, vzt::Format scDepthFormat, vzt::Size2D<uint32_t> scImageSize);

		// Engine configuration
		void setFrameBufferSize(vzt::Size2D<uint32_t> frameBufferSize);

		const vzt::AttachmentSettings& getAttachmentSettings(const vzt::AttachmentHandle& handle) const;
		const vzt::StorageSettings&    getStorageSettings(const vzt::StorageHandle& handle) const;

	  private:
		void sortRenderPasses();
		void reorderRenderPasses();

		vzt::AttachmentHandle generateAttachmentHandle() const;
		vzt::StorageHandle    generateStorageHandle() const;

	  private:
		// TODO: Handle could be shared between render graphs
		static inline std::size_t m_handleCounter = 0;

		std::hash<std::size_t> m_hash{};
		vzt::Size2D<uint32_t>  m_frameBufferSize{};

		std::vector<std::size_t>                     m_sortedRenderPassIndices;
		std::vector<vzt::RenderPassHandler>          m_renderPasses;
		vzt::AttachmentList<vzt::AttachmentSettings> m_attachments;
		vzt::StorageList<vzt::StorageSettings>       m_storages;

		std::optional<vzt::AttachmentHandle> m_backBuffer;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP
