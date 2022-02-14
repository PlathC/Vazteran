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
			savedName = m_name + "ColorIn" + std::to_string(m_colorInputs.size());
		}

		m_colorInputs[attachment] = savedName;
	}

	void RenderPassHandler::addColorOutput(vzt::AttachmentHandle& attachment, const std::string& attachmentName)
	{
		attachment.state++;
		std::string savedName = attachmentName;
		if (savedName.empty())
		{
			savedName = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());
		}

		m_colorOutputs[attachment] = savedName;
	}

	void RenderPassHandler::addStorageInput(const vzt::StorageHandle storage, const std::string& storageName)
	{
		std::string savedName = storageName;
		if (savedName.empty())
		{
			savedName = m_name + "StorageIn" + std::to_string(m_storageInputs.size());
		}

		m_storageInputs[storage] = savedName;
	}

	void RenderPassHandler::addStorageOutput(vzt::StorageHandle& storage, const std::string& storageName)
	{
		storage.state++;
		std::string savedName = storageName;
		if (savedName.empty())
		{
			savedName = m_name + "StorageOut" + std::to_string(m_storageOutput.size());
		}

		m_storageOutput[storage] = savedName;
	}

	void RenderPassHandler::setDepthStencilInput(const vzt::AttachmentHandle depthStencil,
	                                             const std::string&          attachmentName)
	{
		std::string savedName = attachmentName;
		if (savedName.empty())
		{
			savedName = m_name + "DepthIn";
		}

		m_depthInput = {depthStencil, savedName};
	}

	void RenderPassHandler::setDepthStencilOutput(vzt::AttachmentHandle& depthStencil,
	                                              const std::string&     attachmentName)
	{
		depthStencil.state++;
		std::string savedName = attachmentName;
		if (savedName.empty())
		{
			savedName = m_name + "DepthOut";
		}

		m_depthOutput = {depthStencil, savedName};
	}

	void RenderPassHandler::setRenderFunction(vzt::RenderFunction renderFunction)
	{
		m_renderFunction = std::move(renderFunction);
	}

	void RenderPassHandler::setDepthClearFunction(vzt::DepthClearFunction depthClearFunction)
	{
		m_depthClearFunction = std::move(depthClearFunction);
	}

	void RenderPassHandler::setColorClearFunction(vzt::ColorClearFunction colorClearFunction)
	{
		m_colorClearFunction = std::move(colorClearFunction);
	}

	bool RenderPassHandler::isDependingOn(const RenderPassHandler& other) const
	{
		for (const auto& output : m_colorOutputs)
		{
			const auto currentHandle = output.first;
			for (const auto& input : other.m_colorInputs)
			{
				if (input.first == currentHandle)
				{
					return true;
				}
			}
		}

		for (const auto& output : m_storageOutput)
		{
			const auto currentHandle = output.first;
			for (const auto& input : other.m_storageOutput)
			{
				if (input.first == currentHandle)
				{
					return true;
				}
			}
		}

		// Depth attachment
		if (m_depthOutput.has_value())
		{
			const auto currentHandle = m_depthOutput.value().first;
			if (other.m_depthInput.has_value())
			{
				if (other.m_depthInput.value().first == currentHandle)
				{
					return true;
				}
			}
		}

		return false;
	}

	RenderGraph::RenderGraph()  = default;
	RenderGraph::~RenderGraph() = default;

	vzt::AttachmentHandle RenderGraph::addAttachment(const vzt::AttachmentSettings& settings)
	{
		const vzt::AttachmentHandle handle = generateAttachmentHandle();
		m_attachments[handle]              = settings;

		return handle;
	}

	vzt::StorageHandle RenderGraph::addStorage(const vzt::StorageSettings& settings)
	{
		const vzt::StorageHandle handle = generateStorageHandle();
		m_storages[handle]              = settings;

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
		sortRenderPasses();

		std::cout << "Sorted render passes :" << std::endl;
		for (std::size_t i = 0; i < m_sortedRenderPassIndices.size(); i++)
		{
			std::cout << std::to_string(i) << " => " << m_renderPasses[i].m_name << std::endl;
		}
		std::cout << std::endl;

		reorderRenderPasses();

		std::cout << "Reordered render passes :" << std::endl;
		for (std::size_t i = 0; i < m_sortedRenderPassIndices.size(); i++)
		{
			std::cout << std::to_string(i) << " => " << m_renderPasses[i].m_name << std::endl;
		}

		// generateRenderOperation();
	}

	void RenderGraph::setFrameBufferSize(vzt::Size2D<uint32_t> frameBufferSize)
	{
		m_frameBufferSize = frameBufferSize;
		generateRenderOperations();
	}

	void RenderGraph::sortRenderPasses()
	{
		// https://en.wikipedia.org/wiki/Topological_sorting
		m_sortedRenderPassIndices.clear();
		m_sortedRenderPassIndices.reserve(m_sortedRenderPassIndices.size());

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
			for (std::size_t j = 0; j < m_renderPasses.size(); j++)
			{
				if (j == idx)
					continue;

				const auto& renderPass = m_renderPasses[j];
				if (currentRenderPass.isDependingOn(renderPass))
				{
					processNode(j);
				}
			}

			nodeStatus[idx] = 2;
			m_sortedRenderPassIndices.emplace(m_sortedRenderPassIndices.begin(), idx);
			remainingNodes.erase(std::remove(remainingNodes.begin(), remainingNodes.end(), idx), remainingNodes.end());
		};

		while (!remainingNodes.empty())
		{
			processNode(remainingNodes.front());
		}
	}

	void RenderGraph::reorderRenderPasses()
	{
		if (m_sortedRenderPassIndices.size() >= 2)
			return;

		// Based on https://github.com/Themaister/Granite/blob/master/renderer/render_graph.cpp#L2897
		// Expecting that m_sortedRenderPassIndices contains
		// the sorted list of render pass indices.
		std::vector<std::size_t> toProcess;
		toProcess.reserve(m_sortedRenderPassIndices.size());
		std::swap(toProcess, m_sortedRenderPassIndices);

		const auto schedule = [&](unsigned index) {
			// Need to preserve the order of remaining elements.
			m_sortedRenderPassIndices.push_back(toProcess[index]);
			toProcess.erase(toProcess.begin() + index);
		};

		schedule(0);
		while (!toProcess.empty())
		{
			std::size_t bestCandidateIdx  = 0;
			std::size_t bestOverlapFactor = 0;

			for (std::size_t i = 0; i < toProcess.size(); i++)
			{
				std::size_t overlapFactor = 0;
				for (auto it = m_sortedRenderPassIndices.rbegin(); it != m_sortedRenderPassIndices.rend(); ++it)
				{
					if (m_renderPasses[toProcess[i]].isDependingOn(m_renderPasses[*it]))
						break;
					overlapFactor++;
				}

				if (overlapFactor <= bestOverlapFactor)
					continue;

				bool possibleCandidate = true;
				for (std::size_t j = 0; j < i; j++)
				{
					if (m_renderPasses[toProcess[i]].isDependingOn(m_renderPasses[toProcess[j]]))
					{
						possibleCandidate = false;
						break;
					}
				}

				if (!possibleCandidate)
					continue;

				bestCandidateIdx  = i;
				bestOverlapFactor = overlapFactor;
			}

			m_sortedRenderPassIndices.push_back(toProcess[bestCandidateIdx]);
			toProcess.erase(toProcess.begin() + bestCandidateIdx);
		}
	}

	void RenderGraph::generateRenderOperations() {}

	vzt::AttachmentHandle RenderGraph::generateAttachmentHandle() const { return {m_hash(m_handleCounter++), 0}; }
	vzt::StorageHandle    RenderGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), 0}; }

} // namespace vzt
