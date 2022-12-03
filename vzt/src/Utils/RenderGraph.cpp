#include "vzt/Utils/RenderGraph.hpp"

#include <numeric>
#include <stdexcept>

namespace vzt
{
    bool Pass::isDependingOn(const Pass& other) const
    {
        for (const auto& input : m_colorInputs)
        {
            for (const auto& output : other.m_colorOutputs)
            {
                if (output.id == input.id && output.state == input.state)
                    return true;
            }
        }

        for (const auto& input : m_storageInputs)
        {
            for (const auto& output : other.m_storageOutputs)
            {
                if (output.id == input.id && output.state == input.state)
                    return true;
            }
        }

        // Depth attachment
        if (m_depthInput)
        {
            if (other.m_depthOutput)
            {
                if (other.m_depthOutput->id == m_depthInput->id && other.m_depthOutput->state == m_depthInput->state)
                    return true;
            }
        }

        return false;
    }

    Handle RenderGraph::addAttachment(AttachmentBuilder builder)
    {
        const Handle handle          = generateAttachmentHandle();
        m_attachmentBuilders[handle] = std::move(builder);

        return handle;
    }

    Handle RenderGraph::addStorage(StorageBuilder builder)
    {
        const Handle handle       = generateStorageHandle();
        m_storageBuilders[handle] = std::move(builder);

        return handle;
    }

    Pass& RenderGraph::addPass(const std::string& name, View<Queue> queue)
    {
        m_passes.emplace_back(Pass{name, queue});
        return m_passes.back();
    }

    void RenderGraph::setBackBuffer(const Handle handle) { m_backBuffer = handle; }

    bool RenderGraph::isBackBuffer(const vzt::Handle backBufferHandle) const
    {
        if (m_backBuffer)
            return m_backBuffer->id == backBufferHandle.id && m_backBuffer->state == backBufferHandle.state;

        return false;
    }

    void RenderGraph::compile()
    {
        sort();
        reorder();
    }

    Handle RenderGraph::generateAttachmentHandle() const
    {
        return {m_hash(m_handleCounter++), HandleType::Attachment, 0};
    }

    Handle RenderGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), HandleType::Storage, 0}; }

    void RenderGraph::sort()
    {
        // https://en.wikipedia.org/wiki/Topological_sorting
        m_executionOrder.clear();
        m_executionOrder.reserve(m_passes.size());

        // 0: unmarked, 1: temporary marked, 2: permanent mark
        auto nodeStatus     = std::vector<std::size_t>(m_passes.size(), 0);
        auto remainingNodes = std::vector<std::size_t>(m_passes.size());
        std::iota(remainingNodes.begin(), remainingNodes.end(), 0);

        std::function<void(std::size_t)> processNode;
        processNode = [&](std::size_t idx) {
            const std::size_t currentStatus = nodeStatus[idx];

            if (currentStatus == 1)
                throw std::runtime_error("The render graph is cyclic.");
            if (currentStatus == 2)
                return;

            nodeStatus[idx] = 1;

            const auto& currentRenderPass = m_passes[idx];
            for (std::size_t j = 0; j < m_passes.size(); j++)
            {
                if (j == idx)
                    continue;

                const auto& renderPass = m_passes[j];
                if (currentRenderPass.isDependingOn(renderPass))
                    processNode(j);
            }

            nodeStatus[idx] = 2;
            m_executionOrder.emplace_back(idx);
            remainingNodes.erase(std::remove(remainingNodes.begin(), remainingNodes.end(), idx), remainingNodes.end());
        };

        while (!remainingNodes.empty())
        {
            processNode(remainingNodes.front());
        }
    }

    void RenderGraph::reorder()
    {
        if (m_executionOrder.size() <= 2)
            return;

        // Based on https://github.com/Themaister/Granite/blob/master/renderer/render_graph.cpp#L2897

        // Expecting that m_sortedRenderPassIndices contains the sorted list of render pass indices.
        std::vector<std::size_t> toProcess;
        toProcess.reserve(m_executionOrder.size());
        std::swap(toProcess, m_executionOrder);

        const auto schedule = [&](unsigned index) {
            // Need to preserve the order of remaining elements.
            m_executionOrder.push_back(toProcess[index]);
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
                for (auto it = m_executionOrder.rbegin(); it != m_executionOrder.rend(); ++it)
                {
                    if (m_passes[toProcess[i]].isDependingOn(m_passes[*it]))
                        break;

                    overlapFactor++;
                }

                if (overlapFactor <= bestOverlapFactor)
                    continue;

                bool possibleCandidate = true;
                for (std::size_t j = 0; j < i; j++)
                {
                    if (m_passes[toProcess[i]].isDependingOn(m_passes[toProcess[j]]))
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

            m_executionOrder.push_back(toProcess[bestCandidateIdx]);
            toProcess.erase(toProcess.begin() + bestCandidateIdx);
        }
    }
} // namespace vzt
