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
    Pass::Pass(TaskGraph& graph, std::string name, PassType type)
        : m_graph(&graph), m_name(std::move(name)), m_type(type)
    {
    }

    void Pass::addStorageInput(std::string parameter, const Handle& handle, Optional<Range<std::size_t>> range)
    {
        VZT_ASSERT(handle.type == HandleType::Storage);
        m_uses.push_back({
            .handle    = m_graph->readVersion(handle),
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
            .handle = m_graph->readVersion(handle),
            .use    = Use::Indirect,
            .read   = true,
            .range  = range,
        });
    }

    void Pass::addStorageOutput(std::string parameter, const Handle& handle, Optional<Range<std::size_t>> range)
    {
        const bool isImage = handle.type == HandleType::Attachment;
        m_uses.push_back({
            .handle    = m_graph->writeVersion(handle),
            .use       = isImage ? Use::StorageImage : Use::StorageBuffer,
            .write     = true,
            .parameter = std::move(parameter),
            .range     = range,
        });
    }

    void Pass::addStorageInputOutput(std::string parameter, const Handle& handle, Optional<Range<std::size_t>> range)
    {
        const bool isImage = handle.type == HandleType::Attachment;
        m_uses.push_back({
            .handle    = m_graph->writeVersion(handle),
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
            .handle    = m_graph->readVersion(handle),
            .use       = Use::Sampled,
            .read      = true,
            .parameter = std::move(parameter),
        });
    }

    void Pass::addDepthTextureInput(std::string parameter, const Handle& handle)
    {
        assert(handle.type == HandleType::Attachment);
        m_uses.push_back({
            .handle    = m_graph->readVersion(handle),
            .use       = Use::Sampled,
            .read      = true,
            .parameter = std::move(parameter),
            .aspect    = ImageAspect::Depth,
        });
    }

    void ComputePass::setRecordFunction(std::function<void(ComputeContext&)> record)
    {
        m_recordCallback = [this, record = std::move(record)](uint32_t frame, CommandBuffer& commands) {
            commands.bind(m_pipeline, m_pool[frame]);
            ComputeContext context{{commands, *m_graph, m_pool[frame], frame}};
            record(context);
        };
    }

    void GraphicsPass::setRecordFunction(std::function<void(GraphicsContext&)> record)
    {
        m_recordCallback = [this, record = std::move(record)](uint32_t frame, CommandBuffer& commands) {
            commands.bind(m_pipeline, m_pool[frame]);
            GraphicsContext context{{commands, *m_graph, m_pool[frame], frame}, *this};
            if (!m_manualRendering)
                context.begin();
            record(context);
            if (!m_manualRendering)
                context.end();
        };
    }

    Extent2D GraphicsPass::getExtent() const
    {
        for (const ResourceUse& use : m_uses)
        {
            if (use.use != Use::ColorAttachment && use.use != Use::DepthAttachment)
                continue;

            const Extent3D size =
                m_graph->m_attachmentBuilders[use.handle].size.value_or(m_graph->getBackbufferExtent());
            return {size.width, size.height};
        }

        return m_graph->getBackbufferExtent();
    }

    RenderingInfo GraphicsPass::getRenderingInfo(uint32_t frame, Rect2D renderArea) const
    {
        RenderingInfo info{.renderArea = renderArea};

        const CSpan<View<ImageView>> colors = getColorOutputs(frame);
        uint32_t                     color  = 0;
        for (const ResourceUse& use : m_uses)
        {
            if (use.use != Use::ColorAttachment && use.use != Use::DepthAttachment)
                continue;

            const bool isColor = use.use == Use::ColorAttachment;
            const auto layout  = isColor     ? ImageLayout::ColorAttachmentOptimal
                                 : use.write ? ImageLayout::DepthStencilAttachmentOptimal
                                             : ImageLayout::DepthStencilReadOnlyOptimal;

            const RenderingInfo::RenderingAttachment attachment{
                .view       = isColor ? colors[color++] : getDepth(frame),
                .layout     = layout,
                .loadOp     = use.read    ? LoadOp::Load
                              : use.clear ? LoadOp::Clear
                                          : LoadOp::DontCare,
                .clearValue = use.clear.value_or(Vec4(0.f)),
            };

            if (isColor)
                info.colorAttachments.push_back(attachment);
            else
                info.depthAttachment = attachment;
        }

        return info;
    }

    void GraphicsContext::begin(Optional<Rect2D> area) const
    {
        const Extent2D extent = pass.getExtent();
        const Rect2D   r      = area.value_or(Rect2D{0, 0, extent.width, extent.height});

        commands.setViewport({.upperLeftCorner = {static_cast<float>(r.offsetX), static_cast<float>(r.offsetY)},
                              .size            = {static_cast<float>(r.width), static_cast<float>(r.height)}});
        commands.setScissor({.extent = {r.width, r.height}, .offset = {r.offsetX, r.offsetY}});
        commands.beginRendering(pass.getRenderingInfo(frame, r));
    }

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
            m_recordCallback(i, commands);

        if (m_finalBarrier)
        {
            const PassBarrier& final = *m_finalBarrier;
            commands.barrier(PipelineBarrier{final.src.stages,
                                             final.dst.stages,
                                             {{.image     = m_graph->getImage(i, final.handle),
                                               .oldLayout = final.src.layout,
                                               .newLayout = final.dst.layout,
                                               .src       = final.src.access,
                                               .dst       = final.dst.access,
                                               .aspect    = final.aspect}}});
        }
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
            if (uniform.buffer->getFrameNb() != m_graph->getFrameNb())
                throw std::runtime_error(
                    fmt::format("Pass \"{}\": uniform buffer \"{}\" has {} regions, the graph has {} frames.", m_name,
                                uniform.name, uniform.buffer->getFrameNb(), m_graph->getFrameNb()));
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
            if (use.use == Use::Sampled && m_graph->isBackBuffer(use.handle))
                throw std::runtime_error(fmt::format("Pass \"{}\": swapchain images cannot be sampled.", m_name));
        }

        const uint32_t backbufferNb = m_graph->getFrameNb();

        m_pool = DescriptorPool{device, layout, backbufferNb};
        m_pool.allocate(backbufferNb, layout);

        createDescriptors();
    }

    void Pass::resize() { createDescriptors(); }

    void Pass::bindBackbuffer(uint32_t frame, uint32_t image)
    {
        if (m_colorOutputs.empty())
            return;

        const uint32_t outputNb = static_cast<uint32_t>(m_colorOutputs.size() / m_graph->getFrameNb());
        const uint32_t imageNb  = m_graph->getBackbufferImageNb();

        uint32_t output = 0, backbufferUse = 0;
        for (const ResourceUse& use : m_uses)
        {
            if (use.use != Use::ColorAttachment)
                continue;

            if (m_graph->isBackBuffer(use.handle))
                m_colorOutputs[frame * outputNb + output] = m_backbufferViews[backbufferUse++ * imageNb + image];
            output++;
        }
    }

    void Pass::createDescriptors()
    {
        const uint32_t backbufferNb = m_graph->getFrameNb();
        View<Device>   device       = m_graph->getDevice();

        // Views refer to the graph's images, which are recreated on resize.
        // Descriptors point to them, so their storage is reserved before being filled.
        m_textureSaves.clear();
        m_storageImageViews.clear();
        m_colorOutputImageViews.clear();
        m_depthOutputImageViews.clear();
        m_backbufferViews.clear();
        m_colorOutputs.clear();

        // The backbuffer image of a frame changes at each acquisition: one view per swapchain image
        const uint32_t imageNb = m_graph->getBackbufferImageNb();
        m_backbufferViews.reserve(imageNb * m_uses.size());
        for (const ResourceUse& use : m_uses)
        {
            if (use.use == Use::ColorAttachment && m_graph->isBackBuffer(use.handle))
            {
                for (uint32_t i = 0; i < imageNb; i++)
                    m_backbufferViews.emplace_back(device, m_graph->m_externalBackbuffers[i], ImageAspect::Color);
            }
        }
        m_colorOutputs.reserve(backbufferNb * m_uses.size());

        m_textureSaves.reserve(backbufferNb * m_uses.size());
        m_storageImageViews.reserve(backbufferNb * m_uses.size());
        m_colorOutputImageViews.reserve(backbufferNb * m_uses.size());
        m_depthOutputImageViews.reserve(backbufferNb * m_uses.size());

        for (uint32_t i = 0; i < backbufferNb; i++)
        {
            uint32_t          backbufferUse = 0;
            IndexedDescriptor descriptors{};
            for (const PassUniform& uniform : m_uniforms)
                descriptors[uniform.binding] = uniform.buffer->getDescriptor(i);

            for (const ResourceUse& use : m_uses)
            {
                switch (use.use)
                {
                case Use::ColorAttachment:
                    if (m_graph->isBackBuffer(use.handle))
                        m_colorOutputs.emplace_back(m_backbufferViews[backbufferUse++ * imageNb]);
                    else
                        m_colorOutputs.emplace_back(m_colorOutputImageViews.emplace_back(
                            device, m_graph->getImage(i, use.handle), ImageAspect::Color));
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

    ComputePass::ComputePass(TaskGraph& graph, std::string name, Program&& program)
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
            .handle = m_graph->readVersion(handle),
            .use    = Use::DepthAttachment,
            .read   = true,
            .aspect = ImageAspect::Depth,
        });
    }

    void GraphicsPass::setDepthOutput(const Handle& handle, std::string, Optional<float> clear)
    {
        assert(handle.type == HandleType::Attachment);

        m_uses.push_back({
            .handle = m_graph->writeVersion(handle),
            .use    = Use::DepthAttachment,
            .write  = true,
            .clear  = clear ? Optional<Vec4>{Vec4(*clear, 0.f, 0.f, 0.f)} : Optional<Vec4>{},
            .aspect = ImageAspect::Depth,
        });
    }

    void GraphicsPass::setDepthInputOutput(const Handle& handle, std::string)
    {
        assert(handle.type == HandleType::Attachment);

        m_uses.push_back({
            .handle = m_graph->writeVersion(handle),
            .use    = Use::DepthAttachment,
            .read   = true,
            .write  = true,
            .aspect = ImageAspect::Depth,
        });
    }

    void GraphicsPass::addColorOutput(const Handle& handle, std::string, Optional<Vec4> clear, ColorBlend blend)
    {
        assert(handle.type == HandleType::Attachment);

        m_uses.push_back({
            .handle = m_graph->writeVersion(handle),
            .use    = Use::ColorAttachment,
            .write  = true,
            .blend  = blend,
            .clear  = clear,
        });
    }

    void GraphicsPass::addColorInputOutput(const Handle& handle, std::string, std::string, ColorBlend blend)
    {
        assert(handle.type == HandleType::Attachment);

        m_uses.push_back({
            .handle = m_graph->writeVersion(handle),
            .use    = Use::ColorAttachment,
            .read   = true,
            .write  = true,
            .blend  = blend,
        });
    }

    GraphicsPass::GraphicsPass(TaskGraph& graph, std::string name, Program&& program)
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
                const auto format = m_graph->isBackBuffer(use.handle)
                                        ? m_graph->getBackbufferFormat()
                                        : *m_graph->m_attachmentBuilders[use.handle].format;
                m_graphicsPipelineBuilder.addColor(format, use.blend);
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

    Pass::ResourceState TaskGraph::getState(const Pass::ResourceUse& use, PassType type)
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

    TaskGraph::TaskGraph(View<Device> device) : m_device(device) {}

    void TaskGraph::computeBarriers()
    {
        struct Tracked
        {
            Pass::ResourceState state;
            bool                written;
            Pass*               firstPass    = nullptr; // Barrier of the first use of an image
            std::size_t         firstBarrier = 0;
        };
        HandleMap<Tracked> tracked{};
        Pass*              lastBackbufferPass = nullptr;

        for (auto& pass : m_passes)
        {
            pass->m_barriers.clear();
            pass->m_finalBarrier.reset();

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

                if (isBackBuffer(use.handle))
                    lastBackbufferPass = pass.get();

                const auto previous = tracked.find(use.handle);
                if (previous == tracked.end())
                {
                    // First use of the frame: image contents are discarded
                    Tracked first{state, use.write};
                    if (isImage)
                    {
                        pass->m_barriers.push_back({use.handle, use.use, use.aspect, {}, state});
                        first.firstPass    = pass.get();
                        first.firstBarrier = pass->m_barriers.size() - 1;
                    }

                    tracked[use.handle] = first;
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
                last.state   = state;
                last.written = use.write;
            }
        }

        // Targets are shared by every frame: the first use waits for the last use of the previous execution
        for (const auto& [handle, last] : tracked)
        {
            if (!last.firstPass || isBackBuffer(handle))
                continue;

            Pass::ResourceState& src = last.firstPass->m_barriers[last.firstBarrier].src;
            src.stages               = last.state.stages;
            src.access               = last.written ? last.state.access : Access::None;
        }

        if (lastBackbufferPass)
        {
            const Pass::ResourceState& last = tracked.find(*m_backbuffer)->second.state;
            lastBackbufferPass->m_finalBarrier =
                Pass::PassBarrier{*m_backbuffer,
                                  Use::ColorAttachment,
                                  ImageAspect::Color,
                                  last,
                                  {m_backbufferLayout, PipelineStage::BottomOfPipe, Access::None}};
        }
    }

    void TaskGraph::setBackbufferImage(uint32_t frame, uint32_t image)
    {
        m_frameImages[frame] = image;
        for (auto& pass : m_passes)
            pass->bindBackbuffer(frame, image);
    }

    void TaskGraph::setBackbuffer(CSpan<View<DeviceImage>> images, uint32_t frameNb, ImageLayout finalLayout,
                                  Handle handle)
    {
        m_frameNb          = frameNb;
        m_backbufferFormat = images[0]->getFormat();
        m_backbufferExtent = images[0]->getSize();
        m_backbufferLayout = finalLayout;
        m_backbuffer       = handle;

        m_frameImages.assign(frameNb, 0);
        m_externalBackbuffers.assign(images.begin(), images.end());
    }

    void TaskGraph::setBackbuffer(View<DeviceImage> image, ImageLayout finalLayout, Handle handle)
    {
        setBackbuffer(CSpan<View<DeviceImage>>{&image, 1}, 1, finalLayout, handle);
    }

    void RenderGraph::setBackbuffer(View<Swapchain> swapchain, Handle handle)
    {
        std::vector<View<DeviceImage>> images;
        for (uint32_t i = 0; i < swapchain->getImageNb(); ++i)
            images.emplace_back(swapchain->getImage(i));

        setBackbuffer(images, swapchain->getFrameNb(), ImageLayout::PresentSrcKHR, handle);
    }

    Handle TaskGraph::readVersion(const Handle& handle) const
    {
        const auto it = m_versions.find(handle.id);
        return {handle.id, handle.type, it == m_versions.end() ? 0 : it->second};
    }

    Handle TaskGraph::writeVersion(const Handle& handle) { return {handle.id, handle.type, ++m_versions[handle.id]}; }

    Handle TaskGraph::addAttachment(AttachmentBuilder builder)
    {
        const Handle handle          = generateAttachmentHandle();
        m_attachmentBuilders[handle] = std::move(builder);

        return handle;
    }

    Handle TaskGraph::addStorage(StorageBuilder builder)
    {
        const Handle handle       = generateStorageHandle();
        m_storageBuilders[handle] = std::move(builder);

        return handle;
    }

    ComputePass& TaskGraph::addCompute(std::string name, Program&& program)
    {
        ComputePass* pass = new ComputePass(*this, name, std::move(program));
        m_passes.emplace_back(pass);
        return *pass;
    }

    ComputePass& TaskGraph::addCompute(std::string name, std::vector<Shader> shaders)
    {
        ComputePass* pass = new ComputePass(*this, name, Program(m_device, shaders));
        m_passes.emplace_back(pass);
        return *pass;
    }

    ComputePass& TaskGraph::addCompute(std::string name, Shader shader)
    { return addCompute(name, std::vector<Shader>{shader}); }

    GraphicsPass& TaskGraph::addGraphics(std::string name, Program&& program)
    {
        GraphicsPass* pass = new GraphicsPass(*this, name, std::move(program));
        m_passes.emplace_back(pass);
        return *pass;
    }

    GraphicsPass& TaskGraph::addGraphics(std::string name, std::vector<Shader> shaders)
    {
        GraphicsPass* pass = new GraphicsPass(*this, name, Program(m_device, shaders));
        m_passes.emplace_back(pass);
        return *pass;
    }

    // Versions of a handle all refer to the same resource
    bool TaskGraph::isBackBuffer(const Handle handle) const { return m_backbuffer && m_backbuffer->id == handle.id; }

    void TaskGraph::compile()
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

    void TaskGraph::record(uint32_t i, CommandBuffer& commands)
    {
        for (auto& pass : m_passes)
            pass->record(i, commands);
    }

    void TaskGraph::resize(const Extent2D&)
    {
        createRenderTarget();

        for (auto& pass : m_passes)
            pass->resize();
    }

    Handle TaskGraph::generateAttachmentHandle() const
    {
        return {m_hash(m_handleCounter++), HandleType::Attachment, 0};
    }

    Handle TaskGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), HandleType::Storage, 0}; }

    View<DeviceImage> TaskGraph::getImage(uint32_t backbufferId, Handle handle) const
    {
        if (isBackBuffer(handle))
            return m_externalBackbuffers[m_frameImages[backbufferId]];

        const std::size_t handlePhysicalId = m_handleToPhysical.find(handle)->second;
        return m_images[handlePhysicalId];
    }

    View<Buffer> TaskGraph::getStorage(uint32_t backbufferId, Handle handle) const
    {
        const std::size_t handlePhysicalId = m_handleToPhysical.find(handle)->second;
        return m_storages[handlePhysicalId * m_frameNb + backbufferId];
    }

    const AttachmentBuilder& TaskGraph::getConfiguration(Handle handle) { return m_attachmentBuilders[handle]; }

    std::vector<std::size_t> TaskGraph::sort()
    {
        // TaskGraph sorting based on its topology
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

    void TaskGraph::createRenderTarget()
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
            if (!isBackBuffer(handle))
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

                m_images.emplace_back(m_device, imageBuilder);
            }
        }

        for (const auto& [handle, queues] : m_storageBuilders)
        {
            VZT_ASSERT(handle.type == HandleType::Storage);
            m_handleToPhysical[handle] = storageId;
            storageId++;

            const StorageBuilder& storageBuilder = m_storageBuilders[handle];
            for (uint32_t i = 0; i < m_frameNb; i++)
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
