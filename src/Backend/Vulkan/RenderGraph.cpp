#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"

#include <iostream>
#include <numeric>
#include <stack>

namespace vzt
{
	RenderPassHandler::RenderPassHandler(const vzt::RenderGraph* const graph, std::string name,
	                                     vzt::QueueType queueType)
	    : m_graph(graph), m_name(std::move(name)), m_queueType(queueType)
	{
	}

	void RenderPassHandler::addColorInput(const vzt::AttachmentHandle attachment, const std::string& attachmentName)
	{
		AttachmentInfo attachmentInfo;

		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ShaderReadOnlyOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Load;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		attachmentInfo.barrier.layout   = vzt::ImageLayout::ShaderReadOnlyOptimal;
		attachmentInfo.barrier.accesses = vzt::AccessFlag::ShaderRead;
		attachmentInfo.barrier.stages   = vzt::PipelineStage::FragmentShader;
		m_colorInputs[attachment]       = attachmentInfo;
	}

	void RenderPassHandler::addColorOutput(vzt::AttachmentHandle& attachment, const std::string& attachmentName)
	{
		attachment.state++;

		AttachmentInfo attachmentInfo{};
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ColorAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Clear;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::Store;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_colorOutputs[attachment] = attachmentInfo;
	}

	void RenderPassHandler::addColorInputOutput(vzt::AttachmentHandle& attachment, const std::string& inName,
	                                            const std::string& outName)
	{
		attachment.state++;

		AttachmentInfo inAttachmentInfo{};
		inAttachmentInfo.name = inName;
		if (inAttachmentInfo.name.empty())
		{
			inAttachmentInfo.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());
		}

		inAttachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		inAttachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::ColorAttachmentOptimal;
		inAttachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::Load;
		inAttachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::Store;
		inAttachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		inAttachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_colorInputs[attachment] = inAttachmentInfo;

		AttachmentInfo outAttachmentInfo{};
		outAttachmentInfo.name = outName;
		if (outAttachmentInfo.name.empty())
		{
			outAttachmentInfo.name = m_name + "ColorOut" + std::to_string(m_colorInputs.size());
		}

		m_colorOutputs[attachment] = outAttachmentInfo;
	}

	void RenderPassHandler::addStorageInput(const vzt::StorageHandle storage, const std::string& storageName)
	{
		StorageInfo inStorageInfo;
		inStorageInfo.name = storageName;
		if (inStorageInfo.name.empty())
		{
			inStorageInfo.name = m_name + "StorageIn" + std::to_string(m_storageInputs.size());
		}

		inStorageInfo.barrier.accesses = vzt::AccessFlag::ShaderRead;
		if (m_queueType == vzt::QueueType::Compute)
		{
			inStorageInfo.barrier.stages = vzt::PipelineStage::ComputeShader;
		}
		else
		{
			inStorageInfo.barrier.stages = vzt::PipelineStage::FragmentShader;
		}

		m_storageInputs[storage] = inStorageInfo;
	}

	void RenderPassHandler::addStorageOutput(vzt::StorageHandle& storage, const std::string& storageName)
	{
		storage.state++;
		StorageInfo inStorageInfo{};
		inStorageInfo.name = storageName;
		if (inStorageInfo.name.empty())
		{
			inStorageInfo.name = m_name + "StorageOut" + std::to_string(m_storageInputs.size());
		}

		m_storageOutputs[storage] = inStorageInfo;
	}

	void RenderPassHandler::addStorageInputOutput(vzt::StorageHandle& storage, const std::string& inName,
	                                              const std::string& outName)
	{
		storage.state++;

		StorageInfo inStorageInfo{};
		inStorageInfo.name = inName;
		if (inStorageInfo.name.empty())
		{
			inStorageInfo.name = m_name + "In" + std::to_string(m_colorInputs.size());
		}
		inStorageInfo.barrier.accesses = vzt::AccessFlag::ShaderRead | vzt::AccessFlag::ShaderWrite;
		if (m_queueType == vzt::QueueType::Compute)
		{
			inStorageInfo.barrier.stages = vzt::PipelineStage::ComputeShader;
		}
		else
		{
			inStorageInfo.barrier.stages = vzt::PipelineStage::FragmentShader;
		}
		m_storageInputs[storage] = inStorageInfo;

		StorageInfo outAttachmentInfo{};
		outAttachmentInfo.name = outName;
		if (outAttachmentInfo.name.empty())
		{
			outAttachmentInfo.name = m_name + "Out" + std::to_string(m_colorInputs.size());
		}

		m_storageOutputs[storage] = outAttachmentInfo;
	}

	void RenderPassHandler::setDepthStencilInput(const vzt::AttachmentHandle depthStencil,
	                                             const std::string&          attachmentName)
	{
		AttachmentInfo attachmentInfo;
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "DepthIn";
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_depthInput = {depthStencil, attachmentInfo};
	}

	void RenderPassHandler::setDepthStencilOutput(vzt::AttachmentHandle& depthStencil,
	                                              const std::string&     attachmentName)
	{
		depthStencil.state++;

		AttachmentInfo attachmentInfo;
		attachmentInfo.name = attachmentName;
		if (attachmentInfo.name.empty())
		{
			attachmentInfo.name = m_name + "DepthOut";
		}

		attachmentInfo.attachmentUse.initialLayout  = vzt::ImageLayout::Undefined;
		attachmentInfo.attachmentUse.finalLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal;
		attachmentInfo.attachmentUse.loadOp         = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.storeOp        = vzt::StoreOperation::DontCare;
		attachmentInfo.attachmentUse.stencilLoapOp  = vzt::LoadOperation::DontCare;
		attachmentInfo.attachmentUse.stencilStoreOp = vzt::StoreOperation::DontCare;

		m_depthOutput = {depthStencil, attachmentInfo};
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
				if (input.first.id == currentHandle.id && input.first.state == currentHandle.state)
				{
					return true;
				}
			}
		}

		for (const auto& output : m_storageOutputs)
		{
			const auto currentHandle = output.first;
			for (const auto& input : other.m_storageOutputs)
			{
				if (input.first.id == currentHandle.id && input.first.state == currentHandle.state)
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
				const auto otherHandle = other.m_depthInput.value().first;
				if (otherHandle.id == currentHandle.id && otherHandle.state == currentHandle.state)
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
		m_renderPasses.emplace_back(vzt::RenderPassHandler(this, name, queueType));
		return m_renderPasses.back();
	}

	void RenderGraph::setBackBuffer(const vzt::AttachmentHandle backBufferHandle) { m_backBuffer = backBufferHandle; }
	bool RenderGraph::isBackBuffer(const vzt::AttachmentHandle backBufferHandle) const
	{
		return m_backBuffer == backBufferHandle;
	}

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
	}

	void RenderGraph::setFrameBufferSize(vzt::Size2D<uint32_t> frameBufferSize)
	{
		m_frameBufferSize = frameBufferSize;
		// generateRenderOperations();
	}

	const vzt::AttachmentSettings& RenderGraph::getAttachmentSettings(const vzt::AttachmentHandle& handle) const
	{
		for (const auto& savedHandle : m_attachments)
		{
			if (savedHandle.first.id == handle.id)
			{
				return savedHandle.second;
			}
		}
		throw std::runtime_error("Unknown referenced attachment.");
	}

	const vzt::StorageSettings& RenderGraph::getStorageSettings(const vzt::StorageHandle& handle) const
	{
		for (const auto& savedHandle : m_storages)
		{
			if (savedHandle.first.id == handle.id)
			{
				return savedHandle.second;
			}
		}
		throw std::runtime_error("Unknown referenced storage.");
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
				// Try to find the farthest non-depending pass
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

	vzt::AttachmentHandle RenderGraph::generateAttachmentHandle() const { return {m_hash(m_handleCounter++), 0}; }
	vzt::StorageHandle    RenderGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), 0}; }

} // namespace vzt
