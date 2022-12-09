#include "vzt/Utils/RenderGraph.hpp"

#include <bitset>
#include <numeric>
#include <stdexcept>
#include <string>

#include "vzt/Core/Logger.hpp"
#include "vzt/Vulkan/Swapchain.hpp"
#include "vzt/Vulkan/Texture.hpp"

namespace vzt
{
    LambdaRecorder::LambdaRecorder(RecordCallback callback) : m_callback(std::move(callback)) {}

    void LambdaRecorder::record(uint32_t i, const DescriptorSet& set, CommandBuffer& commands)
    {
        if (m_callback)
            m_callback(i, set, commands);
    }

    Pass::Pass(std::string name, View<Queue> queue)
        : m_name(std::move(name)), m_queue(std::move(queue)), m_descriptorLayout(m_queue->getDevice())
    {
    }

    void Pass::addColorInput(uint32_t binding, const Handle& handle, std::string name)
    {
        assert(handle.type == HandleType::Attachment);

        PassAttachment attachment{handle, name};
        attachment.binding = binding;
        if (attachment.name.empty())
            attachment.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());

        attachment.use.finalLayout = ImageLayout::ShaderReadOnlyOptimal;
        attachment.use.usedLayout  = ImageLayout::ShaderReadOnlyOptimal;
        attachment.use.loadOp      = LoadOp::Load;
        attachment.use.storeOp     = StoreOp::DontCare;

        m_colorInputs.emplace_back(attachment);

        m_descriptorLayout.addBinding(binding, DescriptorType::CombinedSampler);
    }

    void Pass::addColorOutput(Handle& handle, std::string name)
    {
        assert(handle.type == HandleType::Attachment);

        handle.state++;
        PassAttachment attachment{handle, name};
        if (attachment.name.empty())
            attachment.name = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());

        attachment.use.usedLayout  = ImageLayout::ColorAttachmentOptimal;
        attachment.use.finalLayout = ImageLayout::ColorAttachmentOptimal;
        attachment.use.loadOp      = LoadOp::Clear;
        attachment.use.storeOp     = StoreOp::Store;

        m_colorOutputs.emplace_back(attachment);
    }

    void Pass::addColorInputOutput(Handle& handle, std::string inName, std::string outName)
    {
        assert(handle.type == HandleType::Attachment);

        PassAttachment inAttachment{handle, inName};
        if (inAttachment.name.empty())
            inAttachment.name = m_name + "ColorIn" + std::to_string(m_colorInputs.size());

        inAttachment.use.finalLayout = ImageLayout::ColorAttachmentOptimal;
        inAttachment.use.usedLayout  = ImageLayout::ColorAttachmentOptimal;
        inAttachment.use.loadOp      = LoadOp::Load;
        inAttachment.use.storeOp     = StoreOp::Store;

        m_colorInputs.emplace_back(inAttachment);

        handle.state++;
        PassAttachment outAttachment{handle, outName};
        if (outAttachment.name.empty())
            outAttachment.name = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());

        outAttachment.use.usedLayout  = ImageLayout::ColorAttachmentOptimal;
        outAttachment.use.finalLayout = ImageLayout::ColorAttachmentOptimal;
        outAttachment.use.loadOp      = LoadOp::Load;
        outAttachment.use.storeOp     = StoreOp::Store;

        m_colorOutputs.emplace_back(outAttachment);
    }

    void Pass::addStorageInput(uint32_t binding, const Handle& handle, std::string name,
                               Optional<Range<std::size_t>> range)
    {
        assert(handle.type == HandleType::Attachment);

        PassStorage storage{handle, name, range, binding};
        if (storage.name.empty())
            storage.name = m_name + "StorageIn" + std::to_string(m_storageInputs.size());

        m_storageInputs.emplace_back(storage);
        m_descriptorLayout.addBinding(binding, DescriptorType::StorageBuffer);
    }

    void Pass::addStorageOutput(Handle& handle, std::string name, Optional<Range<std::size_t>> range)
    {
        handle.state++;

        PassStorage storage{handle, name, range};
        if (storage.name.empty())
            storage.name = m_name + "StorageOut" + std::to_string(m_storageInputs.size());

        m_storageOutputs.emplace_back(storage);
    }

    void Pass::addStorageInputOutput(Handle& handle, std::string inName, std::string outName,
                                     Optional<Range<std::size_t>> range)
    {
        handle.state++;

        PassStorage inStorage{handle, inName, range};
        if (inStorage.name.empty())
            inStorage.name = m_name + "In" + std::to_string(m_colorInputs.size());

        m_storageInputs.emplace_back(inStorage);

        PassStorage outStorage{handle, outName, range};
        if (outStorage.name.empty())
            outStorage.name = m_name + "Out" + std::to_string(m_colorInputs.size());

        m_storageOutputs.emplace_back(outStorage);
    }

    void Pass::setDepthInput(const Handle& handle, std::string name)
    {
        assert(handle.type == HandleType::Attachment);

        PassAttachment attachment{handle, name};
        if (attachment.name.empty())
            attachment.name = m_name + "DepthIn";

        attachment.use.initialLayout  = ImageLayout::Undefined;
        attachment.use.usedLayout     = ImageLayout::DepthStencilAttachmentOptimal;
        attachment.use.finalLayout    = ImageLayout::DepthStencilAttachmentOptimal;
        attachment.use.loadOp         = LoadOp::Load;
        attachment.use.storeOp        = StoreOp::DontCare;
        attachment.use.stencilLoapOp  = LoadOp::Load;
        attachment.use.stencilStoreOp = StoreOp::DontCare;

        m_depthInput = attachment;
    }

    void Pass::setDepthOutput(Handle& handle, std::string name)
    {
        assert(handle.type == HandleType::Attachment);

        handle.state++;

        PassAttachment attachment{handle, name};
        if (attachment.name.empty())
            attachment.name = m_name + "DepthOut";

        attachment.use.usedLayout     = ImageLayout::DepthStencilAttachmentOptimal;
        attachment.use.finalLayout    = ImageLayout::DepthStencilAttachmentOptimal;
        attachment.use.loadOp         = LoadOp::Clear;
        attachment.use.storeOp        = StoreOp::Store;
        attachment.use.stencilLoapOp  = LoadOp::Clear;
        attachment.use.stencilStoreOp = StoreOp::Store;

        attachment.use.clearValue = Vec4{1.f, 0.f, 0.f, 0.f};

        m_depthOutput = attachment;
    }

    void Pass::setRecordFunction(std::unique_ptr<RecordHandler>&& recordCallback)
    {
        m_recordCallback = std::move(recordCallback);
    }

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

    void Pass::record(RenderGraph& graph, uint32_t i, CommandBuffer& commands) const
    {
        for (const auto& input : m_colorInputs)
        {
            if (input.handle.state > 0)
            {
                ImageBarrier barrier;
                barrier.image     = graph.getImage(i, input.handle);
                barrier.oldLayout = input.use.initialLayout;
                barrier.newLayout = input.use.usedLayout;
                barrier.src       = Access::ColorAttachmentWrite;
                barrier.dst       = Access::ShaderRead;
                // barrier.srcQueue
                // barrier.dstQueue
                commands.barrier(PipelineStage::ColorAttachmentOutput, PipelineStage::FragmentShader, barrier);
            }
        }

        if (m_queue->getType() == QueueType::Graphics)
            commands.beginPass(m_renderPass, m_frameBuffers[i]);

        if (m_recordCallback)
            m_recordCallback->record(i, m_pool[i], commands);

        if (m_queue->getType() == QueueType::Graphics)
            commands.endPass();
    }

    void Pass::compile(RenderGraph& graph, Format depthFormat)
    {
        View<Device> device = m_queue->getDevice();

        // If the pass is working with a compute queue, it does not need a render pass
        if (m_queue->getType() == QueueType::Graphics)
        {
            m_renderPass = RenderPass(device);
            for (const auto& output : m_colorOutputs)
            {
                const AttachmentBuilder& attachmentBuilder = graph.m_attachmentBuilders[output.handle];

                AttachmentUse attachmentUse = output.use;
                attachmentUse.format        = attachmentBuilder.format.value_or(graph.m_swapchain->getFormat());

                if (graph.isBackBuffer(output.handle))
                    attachmentUse.finalLayout = ImageLayout::PresentSrcKHR;

                m_renderPass.addColor(attachmentUse);
            }

            if (!m_depthOutput)
                throw std::runtime_error("Graphics pass must have a depth output.");

            AttachmentUse attachmentUse = m_depthOutput->use;
            attachmentUse.format        = depthFormat;
            m_renderPass.setDepth(attachmentUse);

            m_renderPass.compile();

            // Guess render pass extent from its outputs
            Optional<Extent2D> extent;
            const Extent2D     swapchainExtent = graph.m_swapchain->getExtent();
            for (const auto& output : m_colorOutputs)
            {
                const auto& builder = graph.m_attachmentBuilders[output.handle];
                extent              = builder.imageSize.value_or(swapchainExtent);
                break;
            }

            if (!extent && m_depthOutput)
            {
                const auto& builder = graph.m_attachmentBuilders[m_depthOutput->handle];
                extent              = builder.imageSize.value_or(swapchainExtent);
            }

            assert(extent && "Can't guess render pass extent since it has no output.");

            // Create render targets and descriptors for the current pass
            m_descriptorLayout.compile();

            const uint32_t swapchainImageNb = graph.m_swapchain->getImageNb();
            m_pool                          = DescriptorPool{device, m_descriptorLayout, swapchainImageNb};
            m_pool.allocate(swapchainImageNb, m_descriptorLayout);

            m_textureSaves.reserve(m_colorInputs.size() * swapchainImageNb);
            for (uint32_t i = 0; i < swapchainImageNb; i++)
            {
                m_frameBuffers.emplace_back(device, *extent);
                FrameBuffer& frameBuffer = m_frameBuffers.back();

                IndexedDescriptor descriptors{};
                descriptors.reserve(m_colorInputs.size() + m_storageInputs.size());

                // Sampled texture
                for (auto& input : m_colorInputs)
                {
                    View<Image> image = graph.getImage(i, input.handle);

                    m_textureSaves.emplace_back(device, image, SamplerBuilder{});
                    Texture& texture = m_textureSaves.back();

                    descriptors[input.binding] =
                        DescriptorImage{DescriptorType::SampledImage, texture.getView(), texture.getSampler()};
                }

                // SSBO
                for (auto& input : m_storageInputs)
                {
                    View<Buffer> storage = graph.getStorage(i, input.handle);
                    descriptors[input.binding] =
                        DescriptorBuffer{DescriptorType::StorageBuffer, BufferCSpan{storage.get(), storage->size()}};
                }

                if (!descriptors.empty())
                    m_pool.update(i, descriptors);

                // Get attachment from framebuffer
                for (auto& output : m_colorOutputs)
                {
                    const AttachmentBuilder& builder = graph.getConfiguration(output.handle);

                    View<Image> image;
                    if (!builder.format)
                        image = graph.m_swapchain->getImage(i);
                    else
                        image = graph.getImage(i, output.handle);

                    frameBuffer.addAttachment(ImageView{device, image, ImageAspect::Color});
                }

                if (m_depthOutput)
                {
                    frameBuffer.addAttachment(
                        ImageView{device, graph.getImage(i, m_depthOutput->handle), ImageAspect::Depth});
                }

                frameBuffer.compile(m_renderPass);
            }
        }
        else if (m_queue->getType() == QueueType::Compute) {}
    }

    RenderGraph::RenderGraph(View<Swapchain> swapchain) : m_swapchain(std::move(swapchain)) {}

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
        m_passes.emplace_back(new Pass(name, queue));
        return *m_passes.back();
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
        m_executionOrder.clear();
        m_handleToPhysical.clear();

        sort();
        create();

        // Create render passes and their corresponding data such as the FrameBuffer
        // Traverse pass in execution order to fit their id with their ressources
        const auto hardware    = m_swapchain->getDevice()->getHardware();
        const auto depthFormat = hardware.getDepthFormat();
        for (const std::size_t passId : m_executionOrder)
            m_passes[passId]->compile(*this, depthFormat);
    }

    void RenderGraph::record(uint32_t i, CommandBuffer& commands)
    {
        for (std::size_t passId : m_executionOrder)
            m_passes[passId]->record(*this, i, commands);
    }

    Handle RenderGraph::generateAttachmentHandle() const
    {
        return {m_hash(m_handleCounter++), HandleType::Attachment, 0};
    }

    Handle RenderGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), HandleType::Storage, 0}; }

    View<Image> RenderGraph::getImage(uint32_t swapchainImageId, Handle handle) const
    {
        const std::size_t handlePhysicalId = m_handleToPhysical.find(handle)->second;
        return m_images[handlePhysicalId * m_swapchain->getImageNb() + swapchainImageId];
    }

    View<Buffer> RenderGraph::getStorage(uint32_t swapchainImageId, Handle handle) const
    {
        const std::size_t handlePhysicalId = m_handleToPhysical.find(handle)->second;
        return m_storages[handlePhysicalId * m_swapchain->getImageNb() + swapchainImageId];
    }

    const AttachmentBuilder& RenderGraph::getConfiguration(Handle handle) { return m_attachmentBuilders[handle]; }

    void RenderGraph::sort()
    {
        // Graph sorting based on its topology
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

                if (currentRenderPass->isDependingOn(*m_passes[j]))
                    processNode(j);
            }

            nodeStatus[idx] = 2;
            m_executionOrder.emplace_back(idx);
            remainingNodes.erase(std::remove(remainingNodes.begin(), remainingNodes.end(), idx), remainingNodes.end());
        };

        while (!remainingNodes.empty())
            processNode(remainingNodes.front());

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
                    if (m_passes[toProcess[i]]->isDependingOn(*m_passes[*it]))
                        break;

                    overlapFactor++;
                }

                if (overlapFactor <= bestOverlapFactor)
                    continue;

                bool possibleCandidate = true;
                for (std::size_t j = 0; j < i; j++)
                {
                    if (m_passes[toProcess[i]]->isDependingOn(*m_passes[toProcess[j]]))
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

            m_executionOrder.emplace_back(toProcess[bestCandidateIdx]);
            toProcess.erase(toProcess.begin() + bestCandidateIdx);
        }
    }

    void RenderGraph::create()
    {
        m_images.clear();
        m_storages.clear();

        // 1. Get all use handles and find by which queue it is used
        HandleMap<QueueType>   handles{};
        HandleMap<ImageLayout> handlesLastLayout{};
        const auto             add = [&handles](const Pass& pass, const Handle& handle) {
            if (handles.find(handle) == handles.end())
                handles[handle] = QueueType::None;
            handles[handle] |= pass.getQueue()->getType();
        };

        const auto addAttachment = [&add, &handlesLastLayout](const Pass& pass, Pass::PassAttachment& attachment) {
            add(pass, attachment.handle);

            if (handlesLastLayout.find(attachment.handle) != handlesLastLayout.end())
                attachment.use.initialLayout = handlesLastLayout[attachment.handle];

            handlesLastLayout[attachment.handle] = attachment.use.usedLayout;
        };

        for (const std::size_t passId : m_executionOrder)
        {
            Pass& pass = *m_passes[passId];
            for (auto& input : pass.m_colorInputs)
                addAttachment(pass, input);

            for (auto& output : pass.m_colorOutputs)
                addAttachment(pass, output);

            for (auto& input : pass.m_storageInputs)
                add(pass, input.handle);

            for (auto& output : pass.m_storageOutputs)
                add(pass, output.handle);

            if (pass.m_depthInput)
                addAttachment(pass, *pass.m_depthInput);

            if (pass.m_depthOutput)
                addAttachment(pass, *pass.m_depthOutput);
        }

        // 2. Create physical memory (Image, Buffer)
        auto           hardware         = m_swapchain->getDevice()->getHardware();
        const auto     depthFormat      = hardware.getDepthFormat();
        const uint32_t swapchainImageNb = m_swapchain->getImageNb();

        std::size_t imageId   = 0;
        std::size_t storageId = 0;
        for (const auto& [handle, queues] : handles)
        {
            if (handle.type == HandleType::Attachment)
            {
                AttachmentBuilder& attachmentBuilder = m_attachmentBuilders[handle];

                // Swapchain images does not need to be created
                if (attachmentBuilder.format || attachmentBuilder.usage != ImageUsage::ColorAttachment)
                {
                    if (!attachmentBuilder.format && attachmentBuilder.usage == ImageUsage::DepthStencilAttachment)
                        attachmentBuilder.format = depthFormat;

                    m_handleToPhysical[handle] = imageId;
                    imageId++;

                    // clang-format off
                    ImageBuilder builder{
                        attachmentBuilder.imageSize.value_or(m_swapchain->getExtent()), 
                        attachmentBuilder.usage,
                        *attachmentBuilder.format
                    };
                    // clang-format on

                    builder.sampleCount = attachmentBuilder.sampleCount;

                    const std::size_t queueTypeNb = std::bitset<sizeof(QueueType)>(toUnderlying(queues)).count();
                    builder.sharingMode           = queueTypeNb > 1u ? SharingMode::Concurrent : SharingMode::Exclusive;

                    for (uint32_t i = 0; i < swapchainImageNb; i++)
                        m_images.emplace_back(Image{attachmentBuilder.device, builder});
                }
            }
            else if (handle.type == HandleType::Storage)
            {
                m_handleToPhysical[handle] = storageId;
                storageId++;

                const StorageBuilder& storageBuilder = m_storageBuilders[handle];
                for (uint32_t i = 0; i < swapchainImageNb; i++)
                    m_storages.emplace_back(
                        Buffer{storageBuilder.device, storageBuilder.size, vzt::BufferUsage::StorageBuffer});
            }
        }
    }

} // namespace vzt
