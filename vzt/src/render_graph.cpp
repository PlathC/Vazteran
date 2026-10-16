#include "vzt/render_graph.hpp"

#include <bitset>
#include <numeric>
#include <stdexcept>
#include <string>

#include <vulkan/vk_enum_string_helper.h>

#include "vzt/core/assert.hpp"
#include "vzt/core/logger.hpp"
#include "vzt/vulkan/swapchain.hpp"
#include "vzt/vulkan/uniform.hpp"

namespace vzt
{
    LambdaRecorder::LambdaRecorder(RecordCallback callback) : m_callback(std::move(callback)) {}

    void LambdaRecorder::record(uint32_t i, const DescriptorSet& set, CommandBuffer& commands)
    {
        if (m_callback)
            m_callback(i, set, commands);
    }

    Pass::Pass(RenderGraph& graph, std::string name, PassType type)
        : m_graph(&graph), m_name(std::move(name)), m_type(type)
    {
    }

    void Pass::addStorageInput(std::string parameter, const Handle& handle, Optional<Range<std::size_t>> range)
    {
        VZT_ASSERT(handle.type == HandleType::Storage);
        m_uses.push_back({
            .handle    = handle,
            .use       = Use::StorageBuffer,
            .read      = true,
            .parameter = std::move(parameter),
            .range     = range,
        });
    }

    void Pass::addStorageInputIndirect(const Handle& handle, Optional<Range<std::size_t>> range)
    {
        VZT_ASSERT(handle.type == HandleType::Storage);
        m_uses.push_back({
            .handle = handle,
            .use    = Use::Indirect,
            .read   = true,
            .range  = range,
        });
    }

    void Pass::addStorageOutput(std::string parameter, Handle& handle, Optional<Range<std::size_t>> range)
    {
        handle.state++;

        const bool isImage = handle.type == HandleType::Attachment;
        m_uses.push_back({
            .handle    = handle,
            .use       = isImage ? Use::StorageImage : Use::StorageBuffer,
            .write     = true,
            .parameter = std::move(parameter),
            .range     = range,
        });
    }

    void Pass::addStorageInputOutput(std::string parameter, Handle& handle, Optional<Range<std::size_t>> range)
    {
        handle.state++;

        const bool isImage = handle.type == HandleType::Attachment;
        m_uses.push_back({
            .handle    = handle,
            .use       = isImage ? Use::StorageImage : Use::StorageBuffer,
            .read      = true,
            .write     = true,
            .parameter = std::move(parameter),
            .range     = range,
        });
    }

    void Pass::addColorTextureInput(std::string parameter, const Handle& handle)
    {
        assert(handle.type == HandleType::Attachment);
        m_uses.push_back({
            .handle    = handle,
            .use       = Use::Sampled,
            .read      = true,
            .parameter = std::move(parameter),
        });
    }

    void Pass::addDepthTextureInput(std::string parameter, const Handle& handle)
    {
        assert(handle.type == HandleType::Attachment);
        m_uses.push_back({
            .handle    = handle,
            .use       = Use::Sampled,
            .read      = true,
            .parameter = std::move(parameter),
            .aspect    = ImageAspect::Depth,
        });
    }

    void Pass::setRecordFunction(std::unique_ptr<RecordHandler>&& recordCallback)
    { m_recordCallback = std::move(recordCallback); }

    bool Pass::isDependingOn(const Pass& other) const
    {
        // A pass depends on the pass that wrote the version of a resource it reads
        for (const ResourceUse& use : m_uses)
        {
            if (!use.read)
                continue;

            const std::size_t readVersion = use.write ? use.handle.state - 1 : use.handle.state;
            for (const ResourceUse& otherUse : other.m_uses)
            {
                if (otherUse.write && otherUse.handle.id == use.handle.id && otherUse.handle.state == readVersion)
                    return true;
            }
        }

        return false;
    }

    void Pass::record(uint32_t i, CommandBuffer& commands) const
    {
        PipelineBarrier barrier{};
        for (const PassBarrier& passBarrier : m_barriers)
        {
            barrier.src |= passBarrier.src.stages;
            barrier.dst |= passBarrier.dst.stages;

            if (passBarrier.use == Use::StorageBuffer || passBarrier.use == Use::Indirect)
            {
                const View<Buffer> buffer = m_graph->getStorage(i, passBarrier.handle);
                barrier.bufferBarriers.push_back(
                    {{buffer, buffer->size()}, passBarrier.src.access, passBarrier.dst.access});
                continue;
            }

            barrier.imageBarriers.push_back({
                .image      = m_graph->getImage(i, passBarrier.handle),
                .oldLayout  = passBarrier.src.layout,
                .newLayout  = passBarrier.dst.layout,
                .src        = passBarrier.src.access,
                .dst        = passBarrier.dst.access,
                .levelCount = m_graph->getConfiguration(passBarrier.handle).mipLevels,
                .aspect     = passBarrier.aspect,
            });
        }

        if (!m_barriers.empty())
        {
            // First uses of a frame have no previous stage to wait for
            if (barrier.src == PipelineStage::None)
                barrier.src = PipelineStage::TopOfPipe;
            commands.barrier(barrier);
        }

        if (m_recordCallback)
            m_recordCallback->record(i, m_pool[i], commands);
    }

    void Pass::bind(std::string name, const UniformBuffer& buffer) { m_uniforms.push_back({std::move(name), buffer}); }

    void Pass::compileDescriptors(const Program& program, const DescriptorLayout& layout)
    {
        View<Device> device = m_graph->getDevice();

        // Resolve shader parameter names to bindings
        const auto findBinding = [&](const std::string& parameter) -> uint32_t {
            for (const ShaderModule& module : program.getModules())
            {
                const auto& names = module.getShader().bindingNames;
                if (const auto it = names.find(parameter); it != names.end())
                    return it->second;
            }

            throw std::runtime_error(
                fmt::format("Pass \"{}\": the shader has no parameter \"{}\".", m_name, parameter));
        };

        for (PassUniform& uniform : m_uniforms)
        {
            if (uniform.buffer->getFrameNb() != m_graph->getBackbufferNb())
                throw std::runtime_error(
                    fmt::format("Pass \"{}\": uniform buffer \"{}\" has {} regions, the graph has {} frames.", m_name,
                                uniform.name, uniform.buffer->getFrameNb(), m_graph->getBackbufferNb()));
            uniform.binding = findBinding(uniform.name);
        }
        for (ResourceUse& use : m_uses)
            use.binding = use.parameter.empty() ? ~0u : findBinding(use.parameter);

        // Every binding of the pipeline layout must be provided by the graph or by a bind()
        std::unordered_set<uint32_t> provided;
        for (const PassUniform& uniform : m_uniforms)
            provided.insert(uniform.binding);
        for (const ResourceUse& use : m_uses)
            provided.insert(use.binding);

        for (const auto [binding, type] : layout.getBindings())
        {
            if (provided.contains(binding))
                continue;

            std::string name = "?";
            for (const ShaderModule& module : program.getModules())
            {
                for (const auto& [parameter, id] : module.getShader().bindingNames)
                {
                    if (id == binding)
                        name = parameter;
                }
            }

            throw std::runtime_error(fmt::format("Pass \"{}\": shader parameter \"{}\" (binding {}, {}) is not bound.",
                                                 m_name, name, binding, string_VkDescriptorType(toVulkan(type))));
        }

        for (const ResourceUse& use : m_uses)
        {
            if (use.use == Use::Sampled && !m_graph->m_attachmentBuilders[use.handle].format)
                throw std::runtime_error(fmt::format("Pass \"{}\": swapchain images cannot be sampled.", m_name));
        }

        const uint32_t backbufferNb = m_graph->getBackbufferNb();

        m_pool = DescriptorPool{device, layout, backbufferNb};
        m_pool.allocate(backbufferNb, layout);

        createDescriptors();
    }

    void Pass::resize() { createDescriptors(); }

    void Pass::createDescriptors()
    {
        const uint32_t backbufferNb = m_graph->getBackbufferNb();
        View<Device>   device       = m_graph->getDevice();

        // Views refer to the graph's images, which are recreated on resize.
        // Descriptors point to them, so their storage is reserved before being filled.
        m_textureSaves.clear();
        m_storageImageViews.clear();
        m_colorOutputImageViews.clear();
        m_depthOutputImageViews.clear();

        m_textureSaves.reserve(backbufferNb * m_uses.size());
        m_storageImageViews.reserve(backbufferNb * m_uses.size());
        m_colorOutputImageViews.reserve(backbufferNb * m_uses.size());
        m_depthOutputImageViews.reserve(backbufferNb * m_uses.size());

        for (uint32_t i = 0; i < backbufferNb; i++)
        {
            IndexedDescriptor descriptors{};
            for (const PassUniform& uniform : m_uniforms)
                descriptors[uniform.binding] = uniform.buffer->getDescriptor(i);

            for (const ResourceUse& use : m_uses)
            {
                switch (use.use)
                {
                case Use::ColorAttachment:
                    m_colorOutputImageViews.emplace_back(device, m_graph->getImage(i, use.handle), ImageAspect::Color);
                    break;
                case Use::DepthAttachment:
                    m_depthOutputImageViews.emplace_back(device, m_graph->getImage(i, use.handle), ImageAspect::Depth);
                    break;
                case Use::Sampled: {
                    const Texture& texture = m_textureSaves.emplace_back(
                        device, m_graph->getImage(i, use.handle),
                        SamplerBuilder{.filter = Filter::Linear, .mipmapMode = MipmapMode::Nearest});
                    descriptors[use.binding] =
                        DescriptorImage{DescriptorType::CombinedSampler, texture.getView(), texture.getSampler()};
                    break;
                }
                case Use::StorageImage: {
                    const ImageView& view =
                        m_storageImageViews.emplace_back(device, m_graph->getImage(i, use.handle), ImageAspect::Color);
                    descriptors[use.binding] =
                        DescriptorImage{DescriptorType::StorageImage, view, {}, ImageLayout::General};
                    break;
                }
                case Use::StorageBuffer: {
                    const View<Buffer> buffer = m_graph->getStorage(i, use.handle);
                    descriptors[use.binding] =
                        DescriptorBuffer{DescriptorType::StorageBuffer, BufferCSpan{buffer, buffer->size()}};
                    break;
                }
                case Use::Indirect: break;
                }
            }

            descriptors.erase(~0u); // Uses which are not bound to a shader parameter
            if (!descriptors.empty())
                m_pool.update(i, descriptors);
        }
    }

    ComputePass::ComputePass(RenderGraph& graph, std::string name, Program&& program)
        : Pass(graph, std::move(name), PassType::Compute), m_program(std::move(program))
    {
    }

    void ComputePass::compile()
    {
        m_pipeline = ComputePipeline({m_program});
        compileDescriptors(m_program, m_pipeline.getDescriptorLayout());
    }

    void GraphicsPass::setDepthInput(const Handle& handle, std::string)
    {
        assert(handle.type == HandleType::Attachment);
        m_uses.push_back({
            .handle = handle,
            .use    = Use::DepthAttachment,
            .read   = true,
            .aspect = ImageAspect::Depth,
        });
    }

    void GraphicsPass::setDepthOutput(Handle& handle, std::string)
    {
        assert(handle.type == HandleType::Attachment);

        handle.state++;
        m_uses.push_back({
            .handle = handle,
            .use    = Use::DepthAttachment,
            .write  = true,
            .aspect = ImageAspect::Depth,
        });
    }

    void GraphicsPass::setDepthInputOutput(Handle& handle, std::string)
    {
        assert(handle.type == HandleType::Attachment);

        handle.state++;
        m_uses.push_back({
            .handle = handle,
            .use    = Use::DepthAttachment,
            .read   = true,
            .write  = true,
            .aspect = ImageAspect::Depth,
        });
    }

    void GraphicsPass::addColorOutput(Handle& handle, std::string, ColorBlend blend)
    {
        assert(handle.type == HandleType::Attachment);

        handle.state++;
        m_uses.push_back({
            .handle = handle,
            .use    = Use::ColorAttachment,
            .write  = true,
            .blend  = blend,
        });
    }

    void GraphicsPass::addColorInputOutput(Handle& handle, std::string, std::string, ColorBlend blend)
    {
        assert(handle.type == HandleType::Attachment);

        handle.state++;
        m_uses.push_back({
            .handle = handle,
            .use    = Use::ColorAttachment,
            .read   = true,
            .write  = true,
            .blend  = blend,
        });
    }

    GraphicsPass::GraphicsPass(RenderGraph& graph, std::string name, Program&& program)
        : Pass(graph, std::move(name), PassType::Graphics), m_program(std::move(program)),
          m_graphicsPipelineBuilder(m_program)
    {
    }

    void GraphicsPass::compile()
    {
        for (const ResourceUse& use : m_uses)
        {
            if (use.use == Use::ColorAttachment)
            {
                const auto format = m_graph->m_attachmentBuilders[use.handle].format;
                m_graphicsPipelineBuilder.addColor(format.value_or(m_graph->getBackbufferFormat()), use.blend);
            }
            else if (use.use == Use::DepthAttachment)
            {
                m_graphicsPipelineBuilder.setDepth(m_graph->m_device->getHardware().getDepthFormat());
            }
        }

        m_pipeline = GraphicsPipeline(m_graphicsPipelineBuilder);
        compileDescriptors(m_program, m_pipeline.getDescriptorLayout());
    }

    void GraphicsPass::resize() { Pass::resize(); }

    Pass::ResourceState RenderGraph::getState(const Pass::ResourceUse& use, PassType type)
    {
        const PipelineStage shaders = //
            type == PassType::Compute ? PipelineStage::ComputeShader
                                      : PipelineStage::VertexShader | PipelineStage::FragmentShader;

        const Access shaderAccess =
            (use.read ? Access::ShaderRead : Access::None) | (use.write ? Access::ShaderWrite : Access::None);

        switch (use.use)
        {
        case Use::ColorAttachment:
            return {
                ImageLayout::ColorAttachmentOptimal,
                PipelineStage::ColorAttachmentOutput,
                Access::ColorAttachmentRead | Access::ColorAttachmentWrite,
            };
        case Use::DepthAttachment:
            return {
                use.write ? ImageLayout::DepthStencilAttachmentOptimal : ImageLayout::DepthStencilReadOnlyOptimal,
                PipelineStage::EarlyFragmentTests | PipelineStage::LateFragmentTests,
                Access::DepthStencilAttachmentRead | (use.write ? Access::DepthStencilAttachmentWrite : Access::None),
            };
        case Use::Sampled:
            return {
                any(use.aspect & ImageAspect::Depth) ? ImageLayout::DepthStencilReadOnlyOptimal
                                                     : ImageLayout::ShaderReadOnlyOptimal,
                shaders,
                Access::ShaderRead,
            };
        case Use::StorageImage: return {ImageLayout::General, shaders, shaderAccess};
        case Use::StorageBuffer: return {ImageLayout::Undefined, shaders, shaderAccess};
        case Use::Indirect: return {ImageLayout::Undefined, PipelineStage::DrawIndirect, Access::IndirectCommandRead};
        }

        return {};
    }

    RenderGraph::RenderGraph(View<Device> device) : m_device(device) {}

    void RenderGraph::computeBarriers()
    {
        struct Tracked
        {
            Pass::ResourceState state;
            bool                written;
        };
        HandleMap<Tracked> tracked{};

        for (auto& pass : m_passes)
        {
            pass->m_barriers.clear();

            HandleMap<ImageLayout> passLayouts{};
            for (Pass::ResourceUse& use : pass->m_uses)
            {
                const bool               isImage = use.use != Use::StorageBuffer && use.use != Use::Indirect;
                const AttachmentBuilder& builder = m_attachmentBuilders[use.handle];
                if (isImage && any(builder.usage & ImageUsage::DepthStencilAttachment))
                {
                    const bool hasStencilAspect = builder.format && hasStencil(*builder.format);
                    use.aspect = hasStencilAspect ? ImageAspect::Depth | ImageAspect::Stencil : ImageAspect::Depth;
                }

                const Pass::ResourceState state = getState(use, pass->m_type);

                // An image has a single layout during a pass
                if (isImage)
                {
                    const auto [layout, inserted] = passLayouts.emplace(use.handle, state.layout);
                    if (!inserted && layout->second != state.layout)
                    {
                        throw std::runtime_error(
                            fmt::format("Pass \"{}\": an image is used with two different layouts ({} and {}).",
                                        pass->m_name, string_VkImageLayout(toVulkan(layout->second)),
                                        string_VkImageLayout(toVulkan(state.layout))));
                    }
                }

                const auto previous = tracked.find(use.handle);
                if (previous == tracked.end())
                {
                    // First use of the frame: image contents are discarded
                    if (isImage)
                        pass->m_barriers.push_back({use.handle, use.use, use.aspect, {}, state});

                    tracked[use.handle] = {state, use.write};
                    continue;
                }

                Tracked& last = previous->second;
                if (!last.written && !use.write && last.state.layout == state.layout)
                {
                    // Read after read: a later write will have to wait for every reader
                    last.state.stages |= state.stages;
                    continue;
                }

                Pass::ResourceState src = last.state;
                if (!last.written)
                    src.access = Access::None; // Write after read only needs an execution dependency

                pass->m_barriers.push_back({use.handle, use.use, use.aspect, src, state});
                last = {state, use.write};
            }
        }
    }

    void RenderGraph::setBackbuffer(View<DeviceImage> image, ImageLayout finalLayout, Handle handle)
    {
        m_backbufferNb     = 1;
        m_backbufferFormat = image->getFormat();
        m_backbufferExtent = image->getSize();
        m_backbufferLayout = finalLayout;

        m_backbuffer = handle;
        m_externalBackbuffers.emplace_back(image);
    }

    void RenderGraph::setBackbuffer(View<Swapchain> swapchain, const Handle handle)
    {
        m_backbufferNb     = swapchain->getImageNb();
        m_backbufferFormat = swapchain->getFormat();
        m_backbufferExtent = swapchain->getExtent();
        m_backbufferLayout = ImageLayout::PresentSrcKHR;
        m_backbuffer       = handle;

        for (uint32_t i = 0; i < m_backbufferNb; ++i)
            m_externalBackbuffers.emplace_back(swapchain->getImage(i));
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

    ComputePass& RenderGraph::addCompute(std::string name, Program&& program)
    {
        ComputePass* pass = new ComputePass(*this, name, std::move(program));
        m_passes.emplace_back(pass);
        return *pass;
    }

    ComputePass& RenderGraph::addCompute(std::string name, std::vector<Shader> shaders)
    {
        ComputePass* pass = new ComputePass(*this, name, Program(m_device, shaders));
        m_passes.emplace_back(pass);
        return *pass;
    }

    ComputePass& RenderGraph::addCompute(std::string name, Shader shader)
    { return addCompute(name, std::vector<Shader>{shader}); }

    GraphicsPass& RenderGraph::addGraphics(std::string name, Program&& program)
    {
        GraphicsPass* pass = new GraphicsPass(*this, name, std::move(program));
        m_passes.emplace_back(pass);
        return *pass;
    }

    GraphicsPass& RenderGraph::addGraphics(std::string name, std::vector<Shader> shaders)
    {
        GraphicsPass* pass = new GraphicsPass(*this, name, Program(m_device, shaders));
        m_passes.emplace_back(pass);
        return *pass;
    }

    bool RenderGraph::isBackBuffer(const Handle backbufferHandle) const
    {
        if (m_backbuffer)
            return m_backbuffer->id == backbufferHandle.id && m_backbuffer->state == backbufferHandle.state;

        return false;
    }

    void RenderGraph::compile()
    {
        // Back buffer must be set before compiling render graph
        VZT_ASSERT(m_backbuffer);

        const std::vector<std::size_t>     executionOrder = sort();
        std::vector<std::unique_ptr<Pass>> sortedPasses{};
        sortedPasses.reserve(m_passes.size());

        for (std::size_t i = 0; i < m_passes.size(); i++)
            sortedPasses.emplace_back(std::move(m_passes[executionOrder[i]]));
        m_passes = std::move(sortedPasses);

        createRenderTarget();
        computeBarriers();

        // Create render passes and their corresponding data such as the FrameBuffer
        // Traverse pass in execution order to fit their id with their ressources
        for (auto& pass : m_passes)
            pass->compile();
    }

    void RenderGraph::record(uint32_t i, CommandBuffer& commands)
    {
        for (auto& pass : m_passes)
            pass->record(i, commands);
    }

    void RenderGraph::resize(const Extent2D&)
    {
        createRenderTarget();

        for (auto& pass : m_passes)
            pass->resize();
    }

    Handle RenderGraph::generateAttachmentHandle() const
    { return {m_hash(m_handleCounter++), HandleType::Attachment, 0}; }

    Handle RenderGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), HandleType::Storage, 0}; }

    View<DeviceImage> RenderGraph::getImage(uint32_t backbufferId, Handle handle) const
    {
        if (isBackBuffer(handle))
            return m_externalBackbuffers[backbufferId];

        const std::size_t handlePhysicalId = m_handleToPhysical.find(handle)->second;
        return m_images[handlePhysicalId * m_backbufferNb + backbufferId];
    }

    View<Buffer> RenderGraph::getStorage(uint32_t backbufferId, Handle handle) const
    {
        const std::size_t handlePhysicalId = m_handleToPhysical.find(handle)->second;
        return m_storages[handlePhysicalId * m_backbufferNb + backbufferId];
    }

    const AttachmentBuilder& RenderGraph::getConfiguration(Handle handle) { return m_attachmentBuilders[handle]; }

    std::vector<std::size_t> RenderGraph::sort()
    {
        // Graph sorting based on its topology
        // https://en.wikipedia.org/wiki/Topological_sorting
        std::vector<std::size_t> executionOrder{};
        executionOrder.reserve(m_passes.size());

        // 0: unmarked, 1: temporary marked, 2: permanent mark
        auto nodeStatus     = std::vector<std::size_t>(m_passes.size(), 0);
        auto remainingNodes = std::vector<std::size_t>(m_passes.size());
        std::iota(remainingNodes.begin(), remainingNodes.end(), 0);

        std::function<void(std::size_t)> processNode = [&](std::size_t idx) {
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
            executionOrder.emplace_back(idx);
            remainingNodes.erase(std::remove(remainingNodes.begin(), remainingNodes.end(), idx), remainingNodes.end());
        };

        while (!remainingNodes.empty())
            processNode(remainingNodes.front());

        if (executionOrder.size() <= 2)
            return executionOrder;

        // Try to schedule passes based on dependers and dependees
        // Based on https://github.com/Themaister/Granite/blob/master/renderer/render_graph.cpp#L2886

        // Expecting that m_sortedRenderPassIndices contains the sorted list of render pass indices.
        std::vector<std::size_t> toProcess;
        toProcess.reserve(executionOrder.size());
        std::swap(toProcess, executionOrder);

        const auto schedule = [&](unsigned index) {
            // Need to preserve the order of remaining elements.
            executionOrder.push_back(toProcess[index]);
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
                for (auto it = executionOrder.rbegin(); it != executionOrder.rend(); ++it)
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

            executionOrder.emplace_back(toProcess[bestCandidateIdx]);
            toProcess.erase(toProcess.begin() + static_cast<long>(bestCandidateIdx));
        }

        return executionOrder;
    }

    void RenderGraph::createRenderTarget()
    {
        m_images.clear();
        m_storages.clear();
        m_handleToPhysical.clear();

        // Create physical memory (Image, Buffer)
        auto       hardware    = m_device->getHardware();
        const auto depthFormat = hardware.getDepthFormat();

        std::size_t imageId   = 0;
        std::size_t storageId = 0;

        for (const auto& [handle, builder] : m_attachmentBuilders)
        {
            VZT_ASSERT(handle.type == HandleType::Attachment);
            AttachmentBuilder& attachmentBuilder = m_attachmentBuilders[handle];

            // Swapchain images does not need to be created
            if (attachmentBuilder.format || attachmentBuilder.usage != ImageUsage::ColorAttachment)
            {
                if (!attachmentBuilder.format && any(attachmentBuilder.usage & ImageUsage::DepthStencilAttachment))
                    attachmentBuilder.format = depthFormat;

                m_handleToPhysical[handle] = imageId;
                imageId++;

                ImageBuilder imageBuilder = {
                    .size        = attachmentBuilder.size.value_or(m_backbufferExtent),
                    .usage       = attachmentBuilder.usage,
                    .format      = *attachmentBuilder.format,
                    .mipLevels   = attachmentBuilder.mipLevels,
                    .sampleCount = attachmentBuilder.sampleCount,
                    .type        = attachmentBuilder.type,
                    .sharingMode = attachmentBuilder.sharingMode,
                    .tiling      = attachmentBuilder.tiling,
                    .mappable    = attachmentBuilder.mappable,
                };

                imageBuilder.sampleCount = attachmentBuilder.sampleCount;
                imageBuilder.sharingMode = SharingMode::Exclusive;

                for (uint32_t i = 0; i < m_backbufferNb; i++)
                    m_images.emplace_back(m_device, imageBuilder);
            }
        }

        for (const auto& [handle, queues] : m_storageBuilders)
        {
            VZT_ASSERT(handle.type == HandleType::Storage);
            m_handleToPhysical[handle] = storageId;
            storageId++;

            const StorageBuilder& storageBuilder = m_storageBuilders[handle];
            for (uint32_t i = 0; i < m_backbufferNb; i++)
                m_storages.emplace_back(Buffer{
                    m_device,
                    storageBuilder.size,
                    storageBuilder.usage,
                    storageBuilder.location,
                    storageBuilder.mappable,
                });
        }
    }
} // namespace vzt
