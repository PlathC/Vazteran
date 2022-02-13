#ifndef VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP
#define VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP

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

	using AttachmentHandle = std::size_t;
	using StorageHandle    = std::size_t;
	class RenderPassHandler
	{
	  public:
		friend class RenderGraph;

	  public:
		RenderPassHandler() = delete;

		void addAttachmentInput(const vzt::AttachmentHandle attachment, const std::string& attachmentName = "");
		void addColorOutput(const vzt::AttachmentHandle attachment, const std::string& attachmentName = "");

		void addStorageInput(const vzt::StorageHandle attachment, const std::string& storageName = "");
		void addStorageOutput(const vzt::StorageHandle attachment, const std::string& storageName = "");

		void setDepthStencilInput(const vzt::AttachmentHandle attachment, const std::string& attachmentName = "");
		void setDepthStencilOutput(const vzt::AttachmentHandle attachment, const std::string& attachmentName = "");

	  private:
		RenderPassHandler(std::string m_name, vzt::QueueType m_queueType);

	  private:
		std::string    m_name;
		vzt::QueueType m_queueType;

		std::unordered_map<vzt::AttachmentHandle, std::string>       m_colorInputs;
		std::unordered_map<vzt::StorageHandle, std::string>          m_storageInputs;
		std::optional<std::pair<vzt::AttachmentHandle, std::string>> m_depthInput;

		std::unordered_map<vzt::AttachmentHandle, std::string>       m_colorOutputs;
		std::unordered_map<vzt::StorageHandle, std::string>          m_storageOutput;
		std::optional<std::pair<vzt::AttachmentHandle, std::string>> m_depthOutput;
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
		void generateRenderOperations();

		vzt::AttachmentHandle generateAttachmentHandle() const;
		vzt::StorageHandle    generateStorageHandle() const;

	  private:
		static inline std::size_t m_handleCounter = 0;

		std::hash<std::size_t> m_hash{};

		vzt::Size2D<uint32_t> m_frameBufferSize{};

		std::vector<std::size_t>                                           m_sortedRenderPassIndices;
		std::vector<vzt::RenderPassHandler>                                m_renderPasses;
		std::unordered_map<vzt::AttachmentHandle, vzt::AttachmentSettings> m_attachments;
		std::unordered_map<vzt::StorageHandle, vzt::StorageSettings>       m_storages;

		std::optional<vzt::AttachmentHandle> m_backBuffer;
	};
} // namespace vzt

#endif // VAZTERAN_BACKEND_VULKAN_RENDERGRAPH_HPP
