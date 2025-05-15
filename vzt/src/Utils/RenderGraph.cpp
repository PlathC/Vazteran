#include "vzt/Utils/RenderGraph.hpp"

#include <bitset>
#include <numeric>
#include <stdexcept>
#include <string>

#include "vzt/Core/Assert.hpp"
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

    Pass::Pass(RenderGraph& graph, std::string name, PassType type)
        : m_graph(&graph), m_name(std::move(name)), m_type(type), m_descriptorLayout(m_graph->getDevice())
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

        attachment.waitStage    = PipelineStage::ColorAttachmentOutput | PipelineStage::ComputeShader;
        attachment.targetStage  = PipelineStage::FragmentShader | PipelineStage::ComputeShader;
        attachment.waitAccess   = Access::ColorAttachmentWrite | Access::ShaderWrite;
        attachment.targetAccess = Access::ShaderRead;

        m_colorInputs.emplace_back(attachment);

        m_descriptorLayout.addBinding(binding, DescriptorType::CombinedSampler);
    }

    void Pass::addDepthInput(uint32_t binding, const Handle& handle, std::string name)
    {
        assert(handle.type == HandleType::Attachment);

        PassAttachment attachment{handle, name};
        attachment.binding = binding;
        if (attachment.name.empty())
            attachment.name = m_name + "DepthIn" + std::to_string(m_colorInputs.size());

        attachment.use.finalLayout = ImageLayout::ShaderReadOnlyOptimal;
        attachment.use.usedLayout  = ImageLayout::ShaderReadOnlyOptimal;
        attachment.use.loadOp      = LoadOp::Load;
        attachment.use.storeOp     = StoreOp::DontCare;

        attachment.waitStage    = PipelineStage::LateFragmentTests | PipelineStage::ComputeShader;
        attachment.targetStage  = PipelineStage::FragmentShader | PipelineStage::ComputeShader;
        attachment.waitAccess   = Access::DepthStencilAttachmentWrite | Access::ShaderWrite;
        attachment.targetAccess = Access::ShaderRead;
        attachment.aspect       = ImageAspect::Depth;

        m_colorInputs.emplace_back(attachment);

        m_descriptorLayout.addBinding(binding, DescriptorType::CombinedSampler);
    }

    void Pass::addColorOutput(Handle& handle, std::string name, vzt::Vec4 clearColor)
    {
        assert(handle.type == HandleType::Attachment);

        handle.state++;
        PassAttachment attachment{handle, name};
        if (attachment.name.empty())
            attachment.name = m_name + "ColorOut" + std::to_string(m_colorOutputs.size());

        // Attachment is already used as output
        if (handle.state > 1)
        {
            attachment.waitStage    = PipelineStage::ColorAttachmentOutput | PipelineStage::ComputeShader;
            attachment.targetStage  = PipelineStage::FragmentShader | PipelineStage::ComputeShader;
            attachment.waitAccess   = Access::ColorAttachmentWrite | Access::ShaderWrite;
            attachment.targetAccess = Access::ShaderRead;

            attachment.use.loadOp  = LoadOp::Load;
            attachment.use.storeOp = StoreOp::Store;
        }
        else
        {
            attachment.use.loadOp     = LoadOp::Clear;
            attachment.use.storeOp    = StoreOp::Store;
            attachment.use.clearValue = clearColor;
        }

        attachment.use.usedLayout  = ImageLayout::ColorAttachmentOptimal;
        attachment.use.finalLayout = ImageLayout::ColorAttachmentOptimal;

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

        inAttachment.waitStage    = PipelineStage::ColorAttachmentOutput | PipelineStage::ComputeShader;
        inAttachment.targetStage  = PipelineStage::FragmentShader | PipelineStage::ComputeShader;
        inAttachment.waitAccess   = Access::ColorAttachmentWrite | Access::ShaderWrite;
        inAttachment.targetAccess = Access::ShaderRead;

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
        VZT_ASSERT(handle.type == HandleType::Storage);

        PassStorage storage{handle, name, range};
        storage.waitAccess   = Access::ShaderWrite;
        storage.waitStage    = PipelineStage::ComputeShader;
        storage.targetAccess = Access::ShaderRead;
        storage.targetStage  = PipelineStage::VertexShader;
        storage.binding      = binding;
        if (storage.name.empty())
            storage.name = m_name + "StorageIn" + std::to_string(m_storageInputs.size());

        storage.binding = binding;
        m_storageInputs.emplace_back(storage);
        m_descriptorLayout.addBinding(binding, DescriptorType::StorageBuffer);
    }

    void Pass::addStorageInputIndirect(const Handle& handle, std::string name, Optional<Range<std::size_t>> range)
    {
        VZT_ASSERT(handle.type == HandleType::Storage);

        PassStorage storage{handle, name, range};
        storage.waitAccess   = Access::ShaderWrite;
        storage.waitStage    = PipelineStage::ComputeShader;
        storage.targetAccess = Access::IndirectCommandRead;
        storage.targetStage  = PipelineStage::DrawIndirect;
        if (storage.name.empty())
            storage.name = m_name + "StorageInIndirect" + std::to_string(m_storageInputs.size());

        m_storageInputs.emplace_back(storage);
    }

    void Pass::addStorageOutput(uint32_t binding, Handle& handle, std::string name, Optional<Range<std::size_t>> range)
    {
        handle.state++;

        if (handle.type == HandleType::Storage)
        {
            PassStorage storage{handle, name, range};
            storage.targetAccess = Access::ShaderWrite;
            storage.targetStage  = PipelineStage::VertexShader | PipelineStage::ComputeShader;
            storage.waitAccess   = Access::ShaderRead;
            storage.waitStage    = PipelineStage::VertexInput | PipelineStage::ComputeShader;
            if (storage.name.empty())
                storage.name = m_name + "StorageOut" + std::to_string(m_storageInputs.size());

            storage.binding = binding;
            m_storageOutputs.emplace_back(storage);
            m_descriptorLayout.addBinding(binding, DescriptorType::StorageBuffer);
        }
        else if (handle.type == HandleType::Attachment)
        {
            PassAttachment storage{handle, name};
            storage.targetAccess = Access::ShaderWrite;
            storage.targetStage  = PipelineStage::VertexShader | PipelineStage::ComputeShader;
            storage.waitAccess   = Access::ShaderRead;
            storage.waitStage    = PipelineStage::VertexInput | PipelineStage::ComputeShader;
            if (storage.name.empty())
                storage.name = m_name + "StorageImageOut" + std::to_string(m_storageImageOutputs.size());

            storage.use.finalLayout = ImageLayout::General;
            storage.use.usedLayout  = ImageLayout::General;
            // storage.use.loadOp      = LoadOp::Load;
            // storage.use.storeOp     = StoreOp::DontCare;

            storage.binding = binding;
            m_storageImageOutputs.emplace_back(storage);
            m_descriptorLayout.addBinding(binding, DescriptorType::StorageImage);
        }
    }

    void Pass::addStorageInputOutput(uint32_t binding, Handle& handle, std::string inName, std::string outName,
                                     Optional<Range<std::size_t>> range)
    {
        handle.state++;

        if (handle.type == HandleType::Storage)
        {
            PassStorage inStorage{handle, inName, range};
            inStorage.targetAccess = Access::ShaderRead | Access::ShaderWrite;
            inStorage.targetStage  = PipelineStage::VertexShader | PipelineStage::ComputeShader;
            inStorage.waitAccess   = Access::ShaderRead | Access::ShaderWrite;
            inStorage.waitStage    = PipelineStage::VertexInput | PipelineStage::ComputeShader;
            if (inStorage.name.empty())
                inStorage.name = m_name + "StorageIn" + std::to_string(m_storageInputs.size());

            inStorage.binding = binding;
            m_storageInputs.emplace_back(inStorage);

            PassStorage outStorage{handle, outName, range};
            if (outStorage.name.empty())
                outStorage.name = m_name + "StorageOut" + std::to_string(m_storageOutputs.size());

            outStorage.binding = binding;
            m_storageOutputs.emplace_back(outStorage);

            m_descriptorLayout.addBinding(binding, DescriptorType::StorageBuffer);
        }
        else if (handle.type == HandleType::Attachment)
        {
            PassAttachment storage{handle, inName};
            storage.targetAccess = Access::ShaderWrite;
            storage.targetStage  = PipelineStage::VertexShader | PipelineStage::ComputeShader;
            storage.waitAccess   = Access::ShaderRead;
            storage.waitStage    = PipelineStage::VertexInput | PipelineStage::ComputeShader;
            if (storage.name.empty())
                storage.name = m_name + "StorageImageOut" + std::to_string(m_storageImageOutputs.size());

            storage.use.finalLayout = ImageLayout::General;
            storage.use.usedLayout  = ImageLayout::General;
            // storage.use.loadOp      = LoadOp::Load;
            // storage.use.storeOp     = StoreOp::DontCare;

            storage.binding = binding;
            m_storageImageOutputs.emplace_back(storage);
            m_descriptorLayout.addBinding(binding, DescriptorType::StorageImage);
        }
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

    void Pass::setDepthOutput(Handle& handle, std::string name, float depth)
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

        attachment.use.clearValue = Vec4{depth, 0.f, 0.f, 0.f};

        m_depthOutput = attachment;
    }

    void Pass::link(const Pipeline& pipeline)
    {
        const auto& layout = pipeline.getDescriptorLayout();
        for (const auto [id, type] : layout.getBindings())
            m_descriptorLayout.addBinding(id, type);
    }

    void Pass::setRecordFunction(std::unique_ptr<RecordHandler>&& recordCallback)
    {
        m_recordCallback = std::move(recordCallback);
    }

    bool Pass::isDependingOn(const Pass& other) const
    {
        for (const auto& input : m_colorInputs)
        {
            for (const auto& output : other.m_storageImageOutputs)
            {
                if (output.handle.id == input.handle.id && output.handle.state == input.handle.state)
                    return true;
            }

            for (const auto& output : other.m_colorOutputs)
            {
                if (output.handle.id == input.handle.id && output.handle.state == input.handle.state)
                    return true;
            }

            if (other.m_depthOutput)
            {
                if (other.m_depthOutput->handle.id == input.handle.id &&
                    other.m_depthOutput->handle.state == input.handle.state)
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

        for (const auto& currentOutput : m_storageOutputs)
        {
            for (const auto& output : other.m_storageOutputs)
            {
                if (output.handle.id == currentOutput.handle.id && output.handle.state == currentOutput.handle.state)
                    return true;
            }

            for (const auto& output : other.m_storageImageOutputs)
            {
                if (output.handle.id == currentOutput.handle.id && output.handle.state == currentOutput.handle.state)
                    return true;
            }
        }

        for (const auto& currentOutput : m_colorOutputs)
        {
            for (const auto& output : other.m_storageOutputs)
            {
                if (output.handle.id == currentOutput.handle.id && output.handle.state == currentOutput.handle.state)
                    return true;
            }

            for (const auto& output : other.m_storageImageOutputs)
            {
                if (output.handle.id == currentOutput.handle.id && output.handle.state == currentOutput.handle.state)
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

    void Pass::record(uint32_t i, CommandBuffer& commands) const
    {
        for (const auto& input : m_colorInputs)
        {
            if (input.handle.state == 0)
                continue;

            ImageBarrier barrier;
            barrier.image     = m_graph->getImage(i, input.handle);
            barrier.oldLayout = input.use.initialLayout;
            barrier.newLayout = input.use.usedLayout;
            barrier.src       = input.waitAccess;
            barrier.dst       = input.targetAccess;
            barrier.aspect    = input.aspect;

            // TODO: Handle many queues
            // barrier.srcQueue
            // barrier.dstQueue

            commands.barrier(input.waitStage, input.targetStage, barrier);
        }

        for (const auto& input : m_storageInputs)
        {
            if (input.handle.state == 0)
                continue;

            const View<Buffer> buffer = m_graph->getStorage(i, input.handle);

            BufferBarrier barrier;
            barrier.buffer = {buffer, buffer->size()};
            barrier.src    = input.waitAccess;
            barrier.dst    = input.targetAccess;

            // TODO: Handle many queues
            // barrier.srcQueue
            // barrier.dstQueue

            commands.barrier(input.waitStage, input.targetStage, barrier);
        }

        for (const auto& output : m_colorOutputs)
        {
            if (output.handle.state == 0 || output.waitAccess == vzt::Access::None)
                continue;

            const AttachmentBuilder& builder = m_graph->getConfiguration(output.handle);

            ImageBarrier barrier;
            if (!builder.format)
                barrier.image = m_graph->m_swapchain->getImage(i);
            else
                barrier.image = m_graph->getImage(i, output.handle);
            barrier.oldLayout = output.use.initialLayout;
            barrier.newLayout = output.use.usedLayout;
            barrier.src       = output.waitAccess;
            barrier.dst       = output.targetAccess;
            barrier.aspect    = output.aspect;

            // TODO: Handle many queues
            // barrier.srcQueue
            // barrier.dstQueue

            commands.barrier(output.waitStage, output.targetStage, barrier);
        }

        for (const auto& output : m_storageImageOutputs)
        {
            if (output.handle.state == 0 || output.waitAccess == vzt::Access::None)
                continue;

            const AttachmentBuilder& builder = m_graph->getConfiguration(output.handle);

            ImageBarrier barrier;
            if (!builder.format)
                barrier.image = m_graph->m_swapchain->getImage(i);
            else
                barrier.image = m_graph->getImage(i, output.handle);
            barrier.oldLayout = output.use.initialLayout;
            barrier.newLayout = output.use.usedLayout;
            barrier.src       = output.waitAccess;
            barrier.dst       = output.targetAccess;
            barrier.aspect    = output.aspect;

            // TODO: Handle many queues
            // barrier.srcQueue
            // barrier.dstQueue

            commands.barrier(output.waitStage, output.targetStage, barrier);
        }

        if (m_type == PassType::Graphics)
            commands.beginPass(m_renderPass, m_frameBuffers[i]);

        if (m_recordCallback)
            m_recordCallback->record(i, m_pool[i], commands);

        if (m_type == PassType::Graphics)
            commands.endPass();
    }

    void Pass::compile(Format depthFormat)
    {
        View<Device> device = m_graph->getDevice();

        for (auto& input : m_colorInputs)
        {
            const AttachmentBuilder& attachmentBuilder = m_graph->m_attachmentBuilders[input.handle];
            if (input.use.format == vzt::Format::Undefined)
                input.use.format = *attachmentBuilder.format;

            const bool isDepth = vzt::any(attachmentBuilder.usage & ImageUsage::DepthStencilAttachment);
            input.aspect       = isDepth ? ImageAspect::Depth : ImageAspect::Color;
        }

        for (auto& output : m_storageImageOutputs)
        {
            const AttachmentBuilder& attachmentBuilder = m_graph->m_attachmentBuilders[output.handle];
            output.use.format = attachmentBuilder.format.value_or(m_graph->m_swapchain->getFormat());
        }

        for (auto& output : m_colorOutputs)
        {
            const AttachmentBuilder& attachmentBuilder = m_graph->m_attachmentBuilders[output.handle];
            if (output.use.format == vzt::Format::Undefined)
                output.use.format = attachmentBuilder.format.value_or(m_graph->m_swapchain->getFormat());

            if (m_graph->isBackBuffer(output.handle))
                output.use.finalLayout = ImageLayout::PresentSrcKHR;
        }

        // If the pass is working with a compute queue, it does not need a render pass
        if (m_type == PassType::Graphics)
        {
            m_renderPass = RenderPass(device);
            for (const auto& output : m_colorOutputs)
                m_renderPass.addColor(output.use);

            for (const auto& output : m_storageImageOutputs)
            {
                const AttachmentBuilder& attachmentBuilder = m_graph->m_attachmentBuilders[output.handle];

                AttachmentUse attachmentUse = output.use;
                attachmentUse.format        = attachmentBuilder.format.value_or(m_graph->m_swapchain->getFormat());
            }

            if (!m_depthOutput)
                throw std::runtime_error("Graphics pass must have a depth output.");

            AttachmentUse attachmentUse = m_depthOutput->use;
            attachmentUse.format        = depthFormat;
            m_renderPass.setDepth(attachmentUse);

            m_renderPass.compile();

            createRenderObjects();
        }

        // Create descriptors for the current pass
        m_descriptorLayout.compile();

        const uint32_t swapchainImageNb = m_graph->m_swapchain->getImageNb();
        m_pool                          = DescriptorPool{device, m_descriptorLayout, swapchainImageNb};
        m_pool.allocate(swapchainImageNb, m_descriptorLayout);

        createDescriptors();
    }

    void Pass::resize()
    {
        if (m_type == PassType::Graphics)
            createRenderObjects();

        createDescriptors();
    }

    void Pass::createRenderObjects()
    {
        // Guess render pass extent from its outputs
        Optional<Extent2D> extent;
        const Extent2D     swapchainExtent = m_graph->m_swapchain->getExtent();
        for (auto& output : m_colorOutputs)
        {
            const auto& builder = m_graph->m_attachmentBuilders[output.handle];
            extent              = builder.size.value_or(swapchainExtent);
            break;
        }

        if (m_depthOutput)
        {
            const auto& builder = m_graph->m_attachmentBuilders[m_depthOutput->handle];
            if (!extent)
                extent = builder.size.value_or(swapchainExtent);
        }

        assert(extent && "Can't guess render pass extent since it has no output.");

        const uint32_t swapchainImageNb = m_graph->m_swapchain->getImageNb();
        View<Device>   device           = m_graph->getDevice();

        m_frameBuffers.clear();
        m_frameBuffers.reserve(swapchainImageNb);
        m_textureSaves.clear();
        m_textureSaves.reserve(m_colorInputs.size() * swapchainImageNb);
        for (uint32_t i = 0; i < swapchainImageNb; i++)
        {
            m_frameBuffers.emplace_back(device, *extent);
            FrameBuffer& frameBuffer = m_frameBuffers.back();

            // Get attachment from framebuffer
            for (auto& output : m_colorOutputs)
            {
                const AttachmentBuilder& builder = m_graph->getConfiguration(output.handle);

                View<DeviceImage> image;
                if (!builder.format)
                    image = m_graph->m_swapchain->getImage(i);
                else
                    image = m_graph->getImage(i, output.handle);

                frameBuffer.addAttachment(ImageView{device, image, ImageAspect::Color});
            }

            if (m_depthOutput)
            {
                frameBuffer.addAttachment(
                    ImageView{device, m_graph->getImage(i, m_depthOutput->handle), ImageAspect::Depth});
            }

            frameBuffer.compile(m_renderPass);
        }
    }

    void Pass::createDescriptors()
    {
        const uint32_t swapchainImageNb = m_graph->m_swapchain->getImageNb();
        View<Device>   device           = m_graph->getDevice();

        for (uint32_t i = 0; i < swapchainImageNb; i++)
        {
            IndexedDescriptor descriptors{};
            descriptors.reserve(m_colorInputs.size() + m_storageInputs.size());

            // Sampled texture
            for (auto& input : m_colorInputs)
            {
                const AttachmentBuilder& attachmentBuilder = m_graph->m_attachmentBuilders[input.handle];
                if (!attachmentBuilder.format)
                {
                    vzt::logger::error("Swapchain images cannot be used as inputs.");
                    throw std::runtime_error("Swapchain image cannot be inputs.");
                }

                if (input.use.format == vzt::Format::Undefined)
                    input.use.format = *attachmentBuilder.format;

                View<DeviceImage> image = m_graph->getImage(i, input.handle);
                m_textureSaves.emplace_back(
                    device, image, SamplerBuilder{.filter = Filter::Linear, .mipmapMode = MipmapMode::Nearest});
                Texture& texture = m_textureSaves.back();

                descriptors[input.binding] =
                    DescriptorImage{DescriptorType::CombinedSampler, texture.getView(), texture.getSampler()};
            }

            // SSBO
            for (auto& input : m_storageInputs)
            {
                if (input.binding == ~uint32_t(0))
                    continue;

                View<Buffer> storage = m_graph->getStorage(i, input.handle);
                descriptors[input.binding] =
                    DescriptorBuffer{DescriptorType::StorageBuffer, BufferCSpan{storage.get(), storage->size()}};
            }

            for (auto& output : m_storageOutputs)
            {
                View<Buffer> storage = m_graph->getStorage(i, output.handle);

                descriptors[output.binding] =
                    DescriptorBuffer{DescriptorType::StorageBuffer, BufferCSpan{storage, storage->size()}};
            }

            for (auto& output : m_storageImageOutputs)
            {
                View<DeviceImage> image = m_graph->getImage(i, output.handle);
                m_imageViews.emplace_back(device, image, ImageAspect::Color);

                ImageView& imageView = m_imageViews.back();

                descriptors[output.binding] =
                    DescriptorImage{DescriptorType::StorageImage, imageView, {}, output.use.usedLayout};
            }

            if (!descriptors.empty())
                m_pool.update(i, descriptors);
        }
    }

    ComputePass::ComputePass(RenderGraph& graph, std::string name, Program&& program)
        : Pass(graph, std::move(name), PassType::Compute), m_program(std::move(program)), m_pipeline(m_program)
    {
        link(m_pipeline);
    }

    void ComputePass::compile(Format depthFormat)
    {
        Pass::compile(depthFormat);
        m_pipeline.compile();
    }

    GraphicsPass::GraphicsPass(RenderGraph& graph, std::string name, Program&& program)
        : Pass(graph, std::move(name), PassType::Graphics), m_program(std::move(program)), m_pipeline(m_program)
    {
        link(m_pipeline);
    }

    void GraphicsPass::compile(Format depthFormat)
    {
        Pass::compile(depthFormat);
        m_pipeline.compile(m_renderPass);
    }

    void GraphicsPass::resize()
    {
        const Extent2D extent = m_graph->m_swapchain->getExtent();
        m_pipeline.resize(vzt::Viewport{extent});

        Pass::resize();
    }

    RenderGraph::RenderGraph(View<Device> device, View<Swapchain> swapchain)
        : m_device(device), m_swapchain(std::move(swapchain))
    {
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

    Pass& RenderGraph::addPass(std::string name, PassType type)
    {
        m_passes.emplace_back(new Pass(*this, name, type));
        return *m_passes.back();
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
    {
        return addCompute(name, std::vector<Shader>{shader});
    }

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

    void RenderGraph::setBackBuffer(const Handle handle) { m_backBuffer = handle; }

    bool RenderGraph::isBackBuffer(const vzt::Handle backBufferHandle) const
    {
        if (m_backBuffer)
            return m_backBuffer->id == backBufferHandle.id && m_backBuffer->state == backBufferHandle.state;

        return false;
    }

    void RenderGraph::compile()
    {
        const std::vector<std::size_t>     executionOrder = sort();
        std::vector<std::unique_ptr<Pass>> sortedPasses{};
        sortedPasses.reserve(m_passes.size());

        for (std::size_t i = 0; i < m_passes.size(); i++)
            sortedPasses.emplace_back(std::move(m_passes[executionOrder[i]]));
        m_passes = std::move(sortedPasses);

        // Get all use handles and find by which queue it is used
        HandleMap<ImageLayout> handlesLastLayout{};

        const auto addAttachment = [&handlesLastLayout](Pass::PassAttachment& attachment) {
            if (handlesLastLayout.find(attachment.handle) != handlesLastLayout.end())
                attachment.use.initialLayout = handlesLastLayout[attachment.handle];

            handlesLastLayout[attachment.handle] = attachment.use.usedLayout;
        };

        for (auto& pass : m_passes)
        {
            for (auto& input : pass->m_colorInputs)
                addAttachment(input);

            for (auto& output : pass->m_colorOutputs)
                addAttachment(output);

            if (pass->m_depthInput)
                addAttachment(*pass->m_depthInput);

            if (pass->m_depthOutput)
                addAttachment(*pass->m_depthOutput);
        }

        createRenderTarget();

        // Create render passes and their corresponding data such as the FrameBuffer
        // Traverse pass in execution order to fit their id with their ressources
        const auto hardware    = m_swapchain->getDevice()->getHardware();
        const auto depthFormat = hardware.getDepthFormat();
        for (auto& pass : m_passes)
            pass->compile(depthFormat);
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
    {
        return {m_hash(m_handleCounter++), HandleType::Attachment, 0};
    }

    Handle RenderGraph::generateStorageHandle() const { return {m_hash(m_handleCounter++), HandleType::Storage, 0}; }

    View<DeviceImage> RenderGraph::getImage(uint32_t swapchainImageId, Handle handle) const
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
        View<Device>   device           = m_swapchain->getDevice();
        auto           hardware         = device->getHardware();
        const auto     depthFormat      = hardware.getDepthFormat();
        const uint32_t swapchainImageNb = m_swapchain->getImageNb();

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
                    .size        = attachmentBuilder.size.value_or(m_swapchain->getExtent()),
                    .usage       = attachmentBuilder.usage,
                    .format      = *attachmentBuilder.format,
                    .mipLevels   = attachmentBuilder.mipLevels,
                    .layout      = attachmentBuilder.layout,
                    .sampleCount = attachmentBuilder.sampleCount,
                    .type        = attachmentBuilder.type,
                    .sharingMode = attachmentBuilder.sharingMode,
                    .tiling      = attachmentBuilder.tiling,
                    .mappable    = attachmentBuilder.mappable,
                };

                imageBuilder.sampleCount = attachmentBuilder.sampleCount;
                imageBuilder.sharingMode = SharingMode::Exclusive;

                for (uint32_t i = 0; i < swapchainImageNb; i++)
                    m_images.emplace_back(device, imageBuilder);
            }
        }
        for (const auto& [handle, queues] : m_storageBuilders)
        {
            VZT_ASSERT(handle.type == HandleType::Storage);
            m_handleToPhysical[handle] = storageId;
            storageId++;

            const StorageBuilder& storageBuilder = m_storageBuilders[handle];
            for (uint32_t i = 0; i < swapchainImageNb; i++)
                m_storages.emplace_back(Buffer{
                    m_swapchain->getDevice(),
                    storageBuilder.size,
                    storageBuilder.usage,
                    storageBuilder.location,
                    storageBuilder.mappable,
                });
        }
    }

} // namespace vzt
