#ifndef VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP

#include <functional>
#include <optional>

#include <vulkan/vulkan.hpp>

#include "Vazteran/Backend/Vulkan/Attachment.hpp"
#include "Vazteran/Backend/Vulkan/Buffer.hpp"
#include "Vazteran/Backend/Vulkan/CommandPool.hpp"
#include "Vazteran/Backend/Vulkan/Descriptor.hpp"
#include "Vazteran/Backend/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Backend/Vulkan/ImageUtils.hpp"
#include "Vazteran/Backend/Vulkan/RenderPass.hpp"

namespace vzt
{
	// References:
	// Hans-Kristian Arntzen, Render graphs and Vulkan � a deep dive,
	// https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

	enum class QueueType : uint32_t
	{
		Graphic = VK_QUEUE_GRAPHICS_BIT,
		Compute = VK_QUEUE_COMPUTE_BIT
	};
	TO_VULKAN_FUNCTION(QueueType, VkQueueFlagBits)

	struct AttachmentSettings
	{
		vzt::ImageUsage                      usage;
		std::optional<vzt::Format>           format{};    // if unset, use swapchain image formats
		std::optional<vzt::Size2D<uint32_t>> imageSize{}; // if unset, use frame buffer size
		vzt::SampleCount                     sampleCount = vzt::SampleCount::Sample1;
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
			std::size_t operator()(const AttachmentHandle& handle) const { return handle.id; }
		};

		bool operator==(const AttachmentHandle& other) const { return id == other.id; }
	};

	struct StorageHandle
	{
		std::size_t id;        // immutable identifier
		std::size_t state = 0; // Describe the state of the handle

		struct hash
		{
			std::size_t operator()(const StorageHandle& handle) const { return handle.id; }
		};

		bool operator==(const StorageHandle& other) const { return id == other.id; }
	};

	struct ImageBarrier
	{
		vzt::PipelineStage    stages;
		vzt::AttachmentAccess accesses;
		vzt::ImageLayout      layout;
	};

	struct StorageBarrier
	{
		vzt::PipelineStage    stages;
		vzt::AttachmentAccess accesses;
	};

	template <class Type>
	using AttachmentList = std::unordered_map<vzt::AttachmentHandle, Type, vzt::AttachmentHandle::hash>;
	template <class Type>
	using StorageList = std::unordered_map<vzt::StorageHandle, Type, vzt::StorageHandle::hash>;

	using RecordFunction     = std::function<void(uint32_t /* imageId */, const VkCommandBuffer& /*cmd*/,
                                              const std::vector<VkDescriptorSet>& /* engineDescriptorSets */)>;
	using ConfigureFunction  = std::function<void(vzt::PipelineContextSettings /*settings*/)>;
	using DepthClearFunction = std::function<bool(vzt::Vec2* /* value */)>;
	using ColorClearFunction = std::function<bool(uint32_t /* renderTargetIdx */, vzt::Vec4* /* value */)>;
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

		void setRecordFunction(vzt::RecordFunction recordFunction);
		void setConfigureFunction(vzt::ConfigureFunction configureFunction);
		void setDepthClearFunction(vzt::DepthClearFunction depthClearFunction);
		void setColorClearFunction(vzt::ColorClearFunction colorClearFunction);

		bool isDependingOn(const RenderPassHandler& other) const;

		std::unique_ptr<vzt::RenderPass> build(RenderGraph* const correspondingGraph, const vzt::Device* device,
		                                       const uint32_t imageId, const uint32_t imageCount,
		                                       const vzt::Size2D<uint32_t>& targetSize, const vzt::Format targetFormat);

		void render(const uint32_t imageId, const vzt::RenderPass* renderPass, VkCommandBuffer commandBuffer) const;

	  private:
		RenderPassHandler(std::string name, vzt::QueueType queueType);

	  private:
		std::string    m_name;
		vzt::QueueType m_queueType;

		struct AttachmentInfo
		{
			std::string                      name;
			vzt::AttachmentPassUse           attachmentUse{};
			std::optional<vzt::ImageBarrier> barrier;
		};

		struct DepthAttachmentInfo
		{
			std::string                      name;
			vzt::DepthAttachmentPassUse      attachmentUse{};
			std::optional<vzt::ImageBarrier> barrier;
		};

		struct StorageInfo
		{
			std::string                        name;
			std::optional<vzt::StorageBarrier> barrier;
		};

		vzt::AttachmentList<AttachmentInfo>                                  m_colorInputs;
		vzt::StorageList<StorageInfo>                                        m_storageInputs;
		std::optional<std::pair<vzt::AttachmentHandle, DepthAttachmentInfo>> m_depthInput;

		vzt::AttachmentList<AttachmentInfo>                                  m_colorOutputs;
		vzt::StorageList<StorageInfo>                                        m_storageOutputs;
		std::optional<std::pair<vzt::AttachmentHandle, DepthAttachmentInfo>> m_depthOutput;

		vzt::RecordFunction     m_recordFunction;
		vzt::ConfigureFunction  m_configureFunction;
		vzt::DepthClearFunction m_depthClearFunction;
		vzt::ColorClearFunction m_colorClearFunction;

		std::unique_ptr<vzt::DescriptorPool> m_descriptorPool;
		std::optional<vzt::DescriptorLayout> m_descriptorLayout;
		std::unique_ptr<vzt::RenderPass>     m_pass{};
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

		RenderGraph(const RenderGraph&)            = delete;
		RenderGraph& operator=(const RenderGraph&) = delete;

		RenderGraph(RenderGraph&&)            = default;
		RenderGraph& operator=(RenderGraph&&) = default;

		~RenderGraph();

		// User configuration
		vzt::AttachmentHandle addAttachment(const vzt::AttachmentSettings& settings);
		vzt::StorageHandle    addStorage(const vzt::StorageSettings& settings);

		vzt::RenderPassHandler& addPass(const std::string& name, const vzt::QueueType queueType);
		void                    setBackBuffer(const vzt::AttachmentHandle backBufferHandle);

		bool isBackBuffer(const vzt::AttachmentHandle backBufferHandle) const;

		// User information check
		void compile();

		// Engine configuration
		void configure(const vzt::Device* device, const std::vector<VkImage>& swapchainImages,
		               vzt::Size2D<uint32_t> scImageSize, vzt::Format scColorFormat, vzt::Format scDepthFormat);

		void render(const std::size_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete,
		            VkFence inFlightFence);

		vzt::Attachment* getAttachment(const std::size_t imageId, const vzt::AttachmentHandle& handle) const;

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

		std::vector<std::size_t>                                    m_sortedRenderPassIndices;
		std::vector<vzt::RenderPassHandler>                         m_renderPassHandlers;
		std::vector<std::vector<std::unique_ptr<vzt::FrameBuffer>>> m_frameBuffers;
		vzt::AttachmentList<vzt::AttachmentSettings>                m_attachmentsSettings;

		std::vector<vzt::AttachmentList<std::size_t>> m_attachmentsIndices;
		std::vector<std::unique_ptr<vzt::Attachment>> m_attachments;

		vzt::StorageList<vzt::StorageSettings> m_storagesSettings;

		const vzt::Device*                   m_device;
		std::vector<vzt::CommandPool>        m_commandPools;
		std::optional<vzt::AttachmentHandle> m_backBuffer;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP