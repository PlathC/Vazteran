#ifndef VAZTERAN_VULKAN_RENDERGRAPH_HPP
#define VAZTERAN_VULKAN_RENDERGRAPH_HPP

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
	// Hans-Kristian Arntzen, Render graphs and Vulkan — a deep dive,
	// https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

	enum class QueueType : uint32_t
	{
		Graphic = VK_QUEUE_GRAPHICS_BIT,
		Compute = VK_QUEUE_COMPUTE_BIT
	};
	TO_VULKAN_FUNCTION(QueueType, VkQueueFlagBits)

	struct AttachmentSettings
	{
		ImageUsage       usage;
		Optional<Format> format{};    // if unset, use swapchain image formats
		Optional<Uvec2>  imageSize{}; // if unset, use frame buffer size
		SampleCount      sampleCount = vzt::SampleCount::Sample1;
	};

	struct StorageSettings
	{
		std::size_t size;
		BufferUsage usage;
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
		PipelineStage    stages;
		AttachmentAccess accesses;
		ImageLayout      layout;
	};

	struct StorageBarrier
	{
		PipelineStage    stages;
		AttachmentAccess accesses;
	};

	template <class Type>
	using AttachmentList = std::unordered_map<AttachmentHandle, Type, AttachmentHandle::hash>;
	template <class Type>
	using StorageList = std::unordered_map<StorageHandle, Type, StorageHandle::hash>;

	class RenderPassHandler;
	using ConfigureFunction  = std::function<void(const RenderPassHandler&)>;
	using RecordFunction     = std::function<void(uint32_t /* imageId */, VkCommandBuffer /* cmd */,
                                              const std::vector<VkDescriptorSet>& /* engineDescriptorSets */)>;
	using DepthClearFunction = std::function<bool(Vec4* /* value */)>;
	using ColorClearFunction = std::function<bool(uint32_t /* renderTargetIdx */, Vec4* /* value */)>;
	class RenderPassHandler
	{
	  public:
		RenderPassHandler() = delete;

		void addColorInput(const AttachmentHandle attachment, const std::string& attachmentName = "");
		void addColorOutput(AttachmentHandle& attachment, const std::string& attachmentName = "");
		void addColorInputOutput(AttachmentHandle& attachment, const std::string& inName = "",
		                         const std::string& outName = "");

		void addStorageInput(const StorageHandle storage, const std::string& storageName = "");
		void addStorageOutput(StorageHandle& storage, const std::string& storageName = "");
		void addStorageInputOutput(StorageHandle& storage, const std::string& inName = "",
		                           const std::string& outName = "");

		void setDepthStencilInput(const AttachmentHandle depthStencil, const std::string& attachmentName = "");
		void setDepthStencilOutput(AttachmentHandle& depthStencil, const std::string& attachmentName = "");

		void setConfigureFunction(ConfigureFunction configureFunction);
		void setRecordFunction(RecordFunction recordFunction);
		void setDepthClearFunction(DepthClearFunction depthClearFunction);
		void setColorClearFunction(ColorClearFunction colorClearFunction);

		bool isDependingOn(const RenderPassHandler& other) const;

		const Device*           getDevice() const { return m_device; }
		const RenderPass*       getTemplate() const { return m_renderPassTemplate; }
		const DescriptorLayout* getDescriptorLayout() const
		{
			if (m_descriptorLayout)
				return &(*m_descriptorLayout);
			return nullptr;
		}
		uint32_t getOutputAttachmentNb() const { return static_cast<uint32_t>(m_colorOutputs.size()); }
		Uvec2    getExtent() const { return m_extent; }

		friend class RenderGraph;

	  private:
		RenderPassHandler(std::string name, QueueType queueType);

		void configure(RenderGraph* const correspondingGraph, const Device* device, const uint32_t imageCount,
		               Uvec2 targetSize);
		std::unique_ptr<RenderPass> build(const uint32_t imageId);

		void render(const uint32_t imageId, VkCommandBuffer commandBuffer) const;

		std::string m_name;
		QueueType   m_queueType;

		const Device*           m_device             = nullptr;
		const RenderGraph*      m_parent             = nullptr;
		const RenderPass*       m_renderPassTemplate = nullptr;
		Uvec2                   m_extent{};
		RenderPassConfiguration m_configuration{};

		struct AttachmentInfo
		{
			std::string            name;
			AttachmentPassUse      attachmentUse{};
			Optional<ImageBarrier> barrier;
		};

		struct StorageInfo
		{
			std::string              name;
			Optional<StorageBarrier> barrier;
		};

		AttachmentList<AttachmentInfo>                        m_colorInputs;
		StorageList<StorageInfo>                              m_storageInputs;
		Optional<std::pair<AttachmentHandle, AttachmentInfo>> m_depthInput;

		AttachmentList<AttachmentInfo>                        m_colorOutputs;
		StorageList<StorageInfo>                              m_storageOutputs;
		Optional<std::pair<AttachmentHandle, AttachmentInfo>> m_depthOutput;

		RecordFunction     m_recordFunction;
		DepthClearFunction m_depthClearFunction;
		ColorClearFunction m_colorClearFunction;
		ConfigureFunction  m_configureFunction;

		std::unique_ptr<DescriptorPool> m_descriptorPool;
		Optional<DescriptorLayout>      m_descriptorLayout;
		std::unique_ptr<RenderPass>     m_pass{};
	};

	struct PhysicalAttachment
	{
		const AttachmentSettings settings;
	};

	struct PhysicalStorage
	{
		const StorageSettings settings;
		Buffer                buffer;
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
		AttachmentHandle addAttachment(AttachmentSettings settings);
		StorageHandle    addStorage(StorageSettings settings);

		RenderPassHandler& addPass(const std::string& name, const QueueType queueType);
		void               setBackBuffer(const AttachmentHandle backBufferHandle);

		bool isBackBuffer(const AttachmentHandle backBufferHandle) const;

		// User information check
		void compile();

		// Engine configuration
		void configure(const Device* device, const std::vector<VkImage>& swapchainImages, Uvec2 scImageSize,
		               Format scColorFormat, Format scDepthFormat);

		void render(const std::size_t imageId, VkSemaphore imageAvailable, VkSemaphore renderComplete,
		            VkFence inFlightFence);

		const Attachment&         getAttachment(const std::size_t imageId, const AttachmentHandle& handle) const;
		const AttachmentSettings& getAttachmentSettings(const AttachmentHandle& handle) const;
		const StorageSettings&    getStorageSettings(const StorageHandle& handle) const;

	  private:
		void sortRenderPasses();
		void reorderRenderPasses();

		AttachmentHandle generateAttachmentHandle() const;
		StorageHandle    generateStorageHandle() const;

		// TODO: Handle could be shared between render graphs
		static inline std::size_t m_handleCounter = 0;

		std::hash<std::size_t> m_hash{};

		std::vector<std::size_t>              m_sortedRenderPassIndices;
		std::vector<RenderPassHandler>        m_renderPassHandlers;
		std::vector<std::vector<FrameBuffer>> m_frameBuffers;
		AttachmentList<AttachmentSettings>    m_attachmentsSettings;

		std::vector<AttachmentList<std::size_t>> m_attachmentsIndices;
		std::vector<Attachment>                  m_attachments;

		StorageList<StorageSettings> m_storagesSettings;

		const Device*              m_device;
		std::vector<CommandPool>   m_commandPools;
		Optional<AttachmentHandle> m_backBuffer;

		Uvec2 m_lastScImageSize;
	};
} // namespace vzt

#endif // VAZTERAN_VULKAN_RENDERGRAPH_HPP
