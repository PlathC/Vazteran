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
	enum class LoadOperation : uint32_t
	{
		Load     = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD,
		Clear    = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
		DontCare = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE
	};

	enum class StoreOperation : uint32_t
	{
		Store    = VK_ATTACHMENT_STORE_OP_STORE,
		DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		NoneKHR  = VK_ATTACHMENT_STORE_OP_NONE_KHR,
		NoneQCOM = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
		NoneExt  = VK_ATTACHMENT_STORE_OP_NONE_EXT
	};

	struct AttachmentSettings
	{
		std::optional<vzt::Format>        format{};    // if unset, use swapchain image formats
		std::optional<vzt::Size2D<float>> imageSize{}; // if unset, use frame buffer size
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

		void addAttachmentInput(const vzt::AttachmentHandle attachment, const std::string& attachmentName = "");
		void addColorOutput(vzt::AttachmentHandle& attachment, const std::string& attachmentName = "");

		void addStorageInput(const vzt::StorageHandle storage, const std::string& storageName = "");
		void addStorageOutput(vzt::StorageHandle& storage, const std::string& storageName = "");

		void setDepthStencilInput(const vzt::AttachmentHandle depthStencil, const std::string& attachmentName = "");
		void setDepthStencilOutput(vzt::AttachmentHandle& depthStencil, const std::string& attachmentName = "");

		void setRenderFunction(vzt::RenderFunction renderFunction);
		void setDepthClearFunction(vzt::DepthClearFunction depthClearFunction);
		void setColorClearFunction(vzt::ColorClearFunction colorClearFunction);

		bool isDependingOn(const RenderPassHandler& other) const;

	  private:
		RenderPassHandler(std::string name, vzt::QueueType queueType);

	  private:
		std::string    m_name;
		vzt::QueueType m_queueType;

		vzt::AttachmentList<std::string>                             m_colorInputs;
		vzt::StorageList<std::string>                                m_storageInputs;
		std::optional<std::pair<vzt::AttachmentHandle, std::string>> m_depthInput;

		vzt::AttachmentList<std::string>                             m_colorOutputs;
		vzt::StorageList<std::string>                                m_storageOutput;
		std::optional<std::pair<vzt::AttachmentHandle, std::string>> m_depthOutput;

		vzt::RenderFunction     m_renderFunction;
		vzt::DepthClearFunction m_depthClearFunction;
		vzt::ColorClearFunction m_colorClearFunction;
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

		// User information check
		void compile(vzt::Format scColorFormat, vzt::Format scDepthFormat, vzt::Size2D<uint32_t> scImageSize);

		// Engine configuration
		void setFrameBufferSize(vzt::Size2D<uint32_t> frameBufferSize);

	  private:
		void sortRenderPasses();
		void reorderRenderPasses();
		void generateRenderOperations();

		vzt::AttachmentHandle generateAttachmentHandle() const;
		vzt::StorageHandle    generateStorageHandle() const;

	  private:
		// TODO: Handle could be shared between rendergraph
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
