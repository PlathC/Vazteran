#include "vzt/Utils/RenderGraph.hpp"

#include <numeric>
#include <stdexcept>
#include <string>

#include "vzt/Core/Logger.hpp"

namespace vzt
{
    Pass::Pass(std::string name, View<Queue> queue) : m_name(std::move(name)), m_queue(std::move(queue)) {}

    void Pass::addColorInput(const Handle& handle, std::string name)
    {
        if (m_colorInputs.find(PassAttachment{handle}) != m_colorInputs.end())
            throw std::runtime_error("Trying to add several time the same attachment.");

        PassAttachment attachment{handle, name};
        if (attachment.name.empty())
            attachment.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());

        attachment.use.finalLayout = ImageLayout::ShaderReadOnlyOptimal;
        attachment.use.usedLayout  = ImageLayout::ShaderReadOnlyOptimal;
        attachment.use.loadOp      = LoadOp::Load;
        attachment.use.storeOp     = StoreOp::DontCare;

        m_colorInputs.insert(attachment);
    }

    void Pass::addColorOutput(Handle& handle, std::string name)
    {
        if (m_colorOutputs.find(PassAttachment{handle}) != m_colorOutputs.end())
            throw std::runtime_error("Trying to add several time the same attachment.");

        handle.state++;
        PassAttachment attachment{handle, name};
        if (attachment.name.empty())
            attachment.name = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());

        attachment.use.usedLayout  = ImageLayout::ColorAttachmentOptimal;
        attachment.use.finalLayout = ImageLayout::ColorAttachmentOptimal;
        attachment.use.loadOp      = LoadOp::Clear;
        attachment.use.storeOp     = StoreOp::Store;

        m_colorOutputs.insert(attachment);
    }

    void Pass::addColorInputOutput(Handle& handle, std::string inName, std::string outName)
    {
        if (m_colorInputs.find(PassAttachment{handle}) != m_colorInputs.end())
            throw std::runtime_error("Trying to add several time the same attachment.");

        PassAttachment inAttachment{handle, inName};
        if (inAttachment.name.empty())
            inAttachment.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());

        inAttachment.use.finalLayout = ImageLayout::ColorAttachmentOptimal;
        inAttachment.use.usedLayout  = ImageLayout::ColorAttachmentOptimal;
        inAttachment.use.loadOp      = LoadOp::Load;
        inAttachment.use.storeOp     = StoreOp::Store;

        m_colorInputs.insert(inAttachment);

        if (m_colorOutputs.find(PassAttachment{handle}) != m_colorOutputs.end())
            throw std::runtime_error("Trying to add several time the same attachment.");

        handle.state++;
        PassAttachment outAttachment{handle, outName};
        if (outAttachment.name.empty())
            outAttachment.name = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());

        outAttachment.use.usedLayout  = ImageLayout::ColorAttachmentOptimal;
        outAttachment.use.finalLayout = ImageLayout::ColorAttachmentOptimal;
        outAttachment.use.loadOp      = LoadOp::Load;
        outAttachment.use.storeOp     = StoreOp::Store;

        m_colorOutputs.insert(outAttachment);
    }

    void Pass::addStorageInput(const Handle& storage, std::string storageName, Optional<Range<std::size_t>> range) {}
    void Pass::addStorageOutput(const Handle& storage, std::string storageName, Optional<Range<std::size_t>> range) {}
    void Pass::addStorageInputOutput(Handle& storage, std::string inName, std::string outName,
                                     Optional<Range<std::size_t>> range)
    {
    }

    void Pass::setDepthInput(const Handle& depthStencil, std::string attachmentName) {}
    void Pass::setDepthOutput(Handle& depthStencil, std::string attachmentName) {}

    bool Pass::isDependingOn(const Pass& other) const
    {
        for (const auto& input : m_colorInputs)
        {
            for (const auto& output : other.m_colorOutputs)
            {
                if (output.handle.id == input.handle.id && output.handle.state == input.handle.state)
                    return true;
            }
        }

        for (const auto& input : m_storageInputs)
        {
            for (const auto& output : other.m_storageOutputs)
            {
                if (output.handle.id == input.handle.id && output.handle.state == input.handle.state)
                    return true;
            }
        }

        if (m_depthInput)
        {
            if (other.m_depthOutput)
            {
                if (other.m_depthOutput->handle.id == m_depthInput->handle.id &&
                    other.m_depthOutput->handle.state == m_depthInput->handle.state)
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

    Pass& RenderGraph::addPass(std::string name, View<Queue> queue)
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

        logger::info("Render graph execution order");
        for (std::size_t i = 0; i < m_executionOrder.size(); i++)
        {
            std::size_t index = m_executionOrder[i];
            logger::info("{} - {}", index, m_passes[i].m_name);
        }
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

        // Try to fit passes between dependers and dependees
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
