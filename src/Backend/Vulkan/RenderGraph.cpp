#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"

namespace vzt
{
	RenderPassHandler::RenderPassHandler(std::string name, vzt::QueueType queueType)
	    : m_name(std::move(name)), m_queueType(queueType)
	{
	}

	void RenderPassHandler::addAttachmentInput(const vzt::AttachmentHandle attachment,
	                                           const std::string&          attachmentName)
	{
		std::string savedName = attachmentName;
		if (savedName.empty())
		{
			savedName = m_name + "In" + std::to_string(m_colorInputs.size());
		}

		m_colorInputs[attachment] = savedName;
	}

	void RenderPassHandler::addColorOutput(const vzt::AttachmentHandle attachment, const std::string& attachmentName)
	{
		std::string savedName = attachmentName;
		if (savedName.empty())
		{
			savedName = m_name + "Out" + std::to_string(m_colorOutputs.size());
		}

		m_colorOutputs[attachment] = savedName;
	}

	void RenderPassHandler::setDepthStencilInput(const vzt::AttachmentHandle attachment,
	                                             const std::string&          attachmentName)
	{
		std::string savedName = attachmentName;
		if (savedName.empty())
		{
			savedName = m_name + "DepthIn";
		}

		m_depthInput = {attachment, savedName};
	}

	void RenderPassHandler::setDepthStencilOutput(const vzt::AttachmentHandle attachment,
	                                              const std::string&          attachmentName)
	{
		std::string savedName = attachmentName;
		if (savedName.empty())
		{
			savedName = m_name + "DepthOut";
		}

		m_depthOutput = {attachment, savedName};
	}

	RenderGraph::RenderGraph()  = default;
	RenderGraph::~RenderGraph() = default;

	vzt::AttachmentHandle RenderGraph::addAttachment(const vzt::AttachmentSettings& settings)
	{
		const vzt::AttachmentHandle handle = generateAttachmentHandle();
		m_attachments[handle]              = settings;

		return handle;
	}

	vzt::RenderPassHandler& RenderGraph::addPass(const std::string& name, const vzt::QueueType queueType)
	{
		m_renderPasses.emplace_back(vzt::RenderPassHandler(name, queueType));
		return m_renderPasses.back();
	}

	void RenderGraph::setBackBuffer(const vzt::AttachmentHandle backBufferHandle) { m_backBuffer = backBufferHandle; }

	void RenderGraph::compile(vzt::Format scColorFormat, vzt::Format scDepthFormat, vzt::Size2D<uint32_t> scImageSize)
	{

		generateRenderOperation();
	}

	void RenderGraph::setFrameBufferSize(vzt::Size2D<uint32_t> frameBufferSize)
	{
		m_frameBufferSize = frameBufferSize;
		generateRenderOperation();
	}

	vzt::AttachmentHandle RenderGraph::generateAttachmentHandle() const { return m_hash(m_attachments.size()); }

	void RenderGraph::generateRenderOperation() {}
} // namespace vzt
