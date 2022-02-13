#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"

#include <iostream>
#include <numeric>

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
		// https://en.wikipedia.org/wiki/Topological_sorting
		std::vector<std::size_t> sortedRenderPasses;
		sortedRenderPasses.reserve(m_renderPasses.size());

		// 0: unmarked, 1: temporary marked, 2: permanent mark
		std::vector<std::size_t> nodeStatus     = std::vector<std::size_t>(m_renderPasses.size(), 0);
		std::vector<std::size_t> remainingNodes = std::vector<std::size_t>(m_renderPasses.size());
		std::iota(remainingNodes.begin(), remainingNodes.end(), 0);

		std::function<void(std::size_t)> processNode;
		processNode = [&](std::size_t idx) {
			std::size_t currentStatus = nodeStatus[idx];

			if (currentStatus == 1)
				throw std::runtime_error("The render graph is cyclic.");
			if (currentStatus == 2)
				return;

			nodeStatus[idx] = 1;

			const auto& currentRenderPass = m_renderPasses[idx];
			for (const auto& output : currentRenderPass.m_colorOutputs)
			{
				const auto currentHandle = output.first;
				for (std::size_t j = 0; j < m_renderPasses.size(); j++)
				{
					const auto& renderPass = m_renderPasses[j];
					for (const auto& input : renderPass.m_colorInputs)
					{
						if (input.first == currentHandle)
						{
							processNode(j);
						}
					}
				}
			}

			if (currentRenderPass.m_depthOutput.has_value())
			{
				auto currentHandle = currentRenderPass.m_depthOutput.value().first;
				for (std::size_t j = 0; j < m_renderPasses.size(); j++)
				{
					const auto& renderPass = m_renderPasses[j];
					if (renderPass.m_depthInput.has_value())
					{
						if (renderPass.m_depthInput.value().first == currentHandle)
						{
							processNode(j);
						}
					}
				}
			}

			nodeStatus[idx] = 2;
			sortedRenderPasses.emplace(sortedRenderPasses.begin(), idx);
			remainingNodes.erase(std::remove(remainingNodes.begin(), remainingNodes.end(), idx), remainingNodes.end());
		};

		while (!remainingNodes.empty())
		{
			processNode(remainingNodes.front());
		}

		for (std::size_t i = 0; i < sortedRenderPasses.size(); i++)
		{
			std::cout << std::to_string(i) << " => " << m_renderPasses[i].m_name << std::endl;
		}

		// generateRenderOperation();
	}

	void RenderGraph::setFrameBufferSize(vzt::Size2D<uint32_t> frameBufferSize)
	{
		m_frameBufferSize = frameBufferSize;
		generateRenderOperation();
	}

	vzt::AttachmentHandle RenderGraph::generateAttachmentHandle() const { return m_hash(m_attachments.size()); }

	void RenderGraph::generateRenderOperation() {}
} // namespace vzt
