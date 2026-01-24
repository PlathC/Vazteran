#include <vzt/camera.hpp>
#include <vzt/compiler.hpp>
#include <vzt/core/logger.hpp>
#include <vzt/render_graph.hpp>
#include <vzt/vulkan/query_pool.hpp>
#include <vzt/vulkan/surface.hpp>
#include <vzt/vulkan/swapchain.hpp>
#include <vzt/vulkan/uniform.hpp>
#include <vzt/window.hpp>

#include "common/loader.hpp"

struct VertexInput
{
    vzt::Vec3 inPosition;
    vzt::Vec3 inNormal;
};

struct alignas(16) GenerationInput
{
    uint32_t maxInstanceCount;
    uint32_t time;
};

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Deferred + Indirect rendering + Instancing + Compute";

    constexpr uint32_t MaxInstanceCount = 2 << 7;
    constexpr uint32_t WorkGroupSize    = 256;

    auto       window   = vzt::Window{ApplicationName, 1024, 1024};
    auto       instance = vzt::Instance{ApplicationName, window.getConfiguration()};
    const auto surface  = vzt::Surface{window, instance};

    auto deviceBuilder = vzt::DeviceBuilder::standard();
    deviceBuilder.add(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    auto device = instance.getDevice(deviceBuilder, surface);

    auto        hardware = device.getHardware();
    const float period   = hardware.getProperties().limits.timestampPeriod;

    auto swapchain = vzt::Swapchain{device, surface};
    auto compiler  = vzt::Compiler(instance);
    auto graph     = vzt::RenderGraph{device};

    vzt::Mesh                mesh = vzt::readObj("samples/Bunny/Bunny.obj");
    std::vector<VertexInput> vertexInputs;
    vertexInputs.reserve(mesh.vertices.size());

    vzt::Vec3 min{std::numeric_limits<float>::max()}, max{std::numeric_limits<float>::lowest()};
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
    {
        const vzt::Vec3 vertex = mesh.vertices[i];
        vertexInputs.emplace_back(VertexInput{vertex, mesh.normals[i]});
        min = glm::min(min, vertex);
        max = glm::max(max, vertex);
    }

    const auto vertexBuffer = vzt::Buffer::From<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer);
    const auto indexBuffer  = vzt::Buffer::From<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);

    vzt::VertexInputDescription vertexDescription{};
    vertexDescription.add(vzt::VertexBinding::Typed<VertexInput>(0));
    vertexDescription.add(offsetof(VertexInput, inPosition), 0, vzt::Format::R32G32B32SFloat, 0); // Position
    vertexDescription.add(offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0);   // Normal

    auto instancesPosition = graph.addStorage( //
        vzt::StorageBuilder{sizeof(vzt::Vec4f) * MaxInstanceCount, vzt::BufferUsage::StorageBuffer});
    auto drawCommands      = graph.addStorage(vzt::StorageBuilder::fromType<VkDrawIndexedIndirectCommand>(
        vzt::BufferUsage::StorageBuffer | vzt::BufferUsage::IndirectBuffer, vzt::MemoryLocation::Device, true));

    // Instance generation pass
    auto& instanceGeneration = graph.addCompute( //
        "InstanceGeneration", compiler("shaders/deferred/instance_generation.slang", "main"));
    {
        instanceGeneration.getDescriptorLayout().addBinding(0, vzt::DescriptorType::UniformBuffer);
        instanceGeneration.addStorageOutput(1, instancesPosition);
        instanceGeneration.addStorageOutput(2, drawCommands);

        instanceGeneration.setRecordFunction<vzt::LambdaRecorder>( //
            [&](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
                vzt::View<vzt::Buffer> buffer = graph.getStorage(i, drawCommands);

                uint8_t*                           data           = buffer->map();
                const VkDrawIndexedIndirectCommand defaultCommand = {uint32_t(mesh.indices.size()), 0, 0, 0, 0};
                std::memcpy(data, &defaultCommand, sizeof(VkDrawIndexedIndirectCommand));
                buffer->unMap();

                vzt::BufferBarrier barrier{*buffer, vzt::Access::TransferWrite, vzt::Access::ShaderWrite};
                commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader, barrier);

                commands.bind(instanceGeneration.getPipeline(), set);
                commands.dispatch(
                    static_cast<uint32_t>(std::ceil(MaxInstanceCount / static_cast<float>(WorkGroupSize))));
            });
    }

    // Draw geometry pass
    auto position =
        graph.addAttachment({.usage = vzt::ImageUsage::ColorAttachment, .format = vzt::Format::R32G32B32A32SFloat});
    auto normal =
        graph.addAttachment({.usage = vzt::ImageUsage::ColorAttachment, .format = vzt::Format::R8G8B8A8SNorm});
    auto depth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry = graph.addGraphics("Geometry", compiler("shaders/deferred/triangle.slang"));
    {
        geometry.getDescriptorLayout().addBinding(0, vzt::DescriptorType::UniformBuffer);
        geometry.addStorageInput(1, instancesPosition);
        geometry.addStorageInputIndirect(drawCommands);
        geometry.addColorOutput(position);
        geometry.addColorOutput(normal);
        geometry.setDepthOutput(depth);

        auto& pipelineBuilder = geometry.getBuilder();
        pipelineBuilder.set(vertexDescription);
        pipelineBuilder.set(vzt::Rasterization{.cullMode = vzt::CullMode::None});

        geometry.setRecordFunction<vzt::LambdaRecorder>(
            [&](uint32_t frame, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
                const auto extent = graph.getBackbufferExtent();
                commands.setViewport(vzt::Viewport{.size = {extent.width, extent.height}});
                commands.setScissor(vzt::Scissor{.extent = extent});

                auto colorViews = geometry.getColorOutputs(frame);
                auto depthView  = geometry.getDepth(frame);
                commands.beginRendering({
                    .renderArea       = {0, 0, extent.width, extent.height},
                    .colorAttachments = {{
                                             .view       = colorViews[0],
                                             .layout     = vzt::ImageLayout::ColorAttachmentOptimal,
                                             .loadOp     = vzt::LoadOp::Clear,
                                             .storeOp    = vzt::StoreOp::Store,
                                             .clearValue = vzt::Vec4(0.f),
                                         },
                                         {
                                             .view       = colorViews[1],
                                             .layout     = vzt::ImageLayout::ColorAttachmentOptimal,
                                             .loadOp     = vzt::LoadOp::Clear,
                                             .storeOp    = vzt::StoreOp::Store,
                                             .clearValue = vzt::Vec4(0.f),
                                         }},
                    .depthAttachment =
                        vzt::RenderingInfo::RenderingAttachment{
                            .view       = depthView,
                            .layout     = vzt::ImageLayout::DepthStencilAttachmentOptimal,
                            .clearValue = vzt::Vec4(1.f, 0.f, 0.f, 0.f),
                        },
                });

                commands.bind(geometry.getPipeline(), set);
                commands.bindVertexBuffer(vertexBuffer);
                commands.bindIndexBuffer(indexBuffer, 0);

                const vzt::View<vzt::Buffer> buffer = graph.getStorage(frame, drawCommands);
                commands.drawIndexedIndirect(*buffer, 1, 0);

                commands.endRendering();
            });
    }

    auto composed      = graph.addAttachment({vzt::ImageUsage::ColorAttachment});
    auto composedDepth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

    auto& shading = graph.addGraphics("Shading", compiler("shaders/deferred/deferred_blinn_phong.slang"));
    {
        shading.addColorTextureInput(0, position);
        shading.addColorTextureInput(1, normal);
        shading.addColorOutput(composed, "");
        shading.setDepthOutput(composedDepth);

        auto& builder = shading.getBuilder();
        builder.set(
            vzt::Rasterization{.cullMode = vzt::CullMode::Front, .frontFace = vzt::FrontFace::CounterClockwise});

        shading.setRecordFunction<vzt::LambdaRecorder>(
            [&](uint32_t frame, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
                const auto extent = graph.getBackbufferExtent();
                commands.setViewport(vzt::Viewport{.size = {extent.width, extent.height}});
                commands.setScissor(vzt::Scissor{.extent = extent});

                auto colorViews = shading.getColorOutputs(frame);
                auto depthView  = shading.getDepth(frame);
                commands.beginRendering({
                    .renderArea       = {0, 0, extent.width, extent.height},
                    .colorAttachments = {{
                        .view       = colorViews[0],
                        .layout     = vzt::ImageLayout::ColorAttachmentOptimal,
                        .loadOp     = vzt::LoadOp::Clear,
                        .storeOp    = vzt::StoreOp::Store,
                        .clearValue = vzt::Vec4(1.f, 0.91f, 0.69f, 1.f),
                    }},
                    .depthAttachment =
                        vzt::RenderingInfo::RenderingAttachment{
                            .view       = depthView,
                            .layout     = vzt::ImageLayout::DepthStencilAttachmentOptimal,
                            .clearValue = vzt::Vec4(1.f, 0.f, 0.f, 0.f),
                        },
                });

                commands.bind(shading.getPipeline(), set);
                commands.draw(3);

                commands.endRendering();
            });
    }

    graph.setBackbuffer(swapchain, composed);
    graph.compile();

    const vzt::QueryPool queryPool = {
        device,
        vzt::QueryType::Timestamp,
        swapchain.getImageNb() * (graph.size() + 1) * 2,
    };

    // Initialize buffer with default values
    const uint32_t     frameNb       = swapchain.getImageNb();
    vzt::UniformBuffer modelsUbo     = vzt::UniformBuffer(device, sizeof(vzt::Mat4) * 3, frameNb, true);
    vzt::UniformBuffer generationUbo = vzt::UniformBuffer(device, sizeof(GenerationInput), frameNb, true);

    // Assign buffer parts to their respective image
    vzt::DescriptorPool& generationDescriptorPool = instanceGeneration.getDescriptorPool();
    vzt::DescriptorPool& geometryDescriptorPool   = geometry.getDescriptorPool();

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        generationDescriptorPool.update(i, {{0, generationUbo.getDescriptor(i)}});
        geometryDescriptorPool.update(i, {{0, modelsUbo.getDescriptor(i)}});
    }

    // Compute AABB to place camera in front of the model
    vzt::Vec3 minimum{std::numeric_limits<float>::max()};
    vzt::Vec3 maximum{std::numeric_limits<float>::lowest()};
    for (vzt::Vec3& vertex : mesh.vertices)
    {
        minimum = glm::min(minimum, vertex);
        maximum = glm::max(maximum, vertex);
    }

    vzt::Camera camera{};
    camera.up    = vzt::Vec3(0.f, 0.f, 1.f);
    camera.front = vzt::Vec3(0.f, 1.f, 0.f);
    camera.right = vzt::Vec3(1.f, 0.f, 0.f);

    camera.aspectRatio = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());

    const vzt::Vec3 target   = (minimum + maximum) * .5f;
    const float     bbRadius = glm::compMax(glm::abs(maximum - target));

    std::vector<uint64_t> times{};
    times.resize((graph.size() + 1) * swapchain.getImageNb() * 2);
    bool hasBenchmark = false;

    // Actual rendering
    while (window.update())
    {
        const auto& inputs = window.getInputs();
        if (inputs.windowResized)
            swapchain.recreate();

        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        const uint32_t startId = submission->imageId * (graph.size() + 1) * 2;
        if (hasBenchmark)
        {
            queryPool.getResults(startId, (graph.size() + 1) * 2,
                                 vzt::Span<uint64_t>(times.data() + startId, (graph.size() + 1) * 2), sizeof(uint64_t),
                                 vzt::QueryResultFlag::N64 | vzt::QueryResultFlag::Wait);

            for (uint32_t i = 0; i < graph.size(); i++)
            {
                auto& pass = graph[i];

                const float ms =
                    (static_cast<float>(times[startId + i * 2 + 1] - times[startId + i * 2 + 0]) * period) / 1000000.f;

                vzt::logger::info("{}: {}ms", pass->getName(), ms);
            }

            const float ms =
                (static_cast<float>(times[startId + graph.size() * 2 + 1] - times[startId + graph.size() * 2 + 0]) *
                 period) /
                1000000.f;
            vzt::logger::info("Total: {}ms", ms);
        }
        hasBenchmark |= submission->imageId == (swapchain.getImageNb() - 1);

        // Per frame update
        vzt::Quat orientation = {1.f, 0.f, 0.f, 0.f};

        float t = std::fmod(static_cast<float>(inputs.time) * 1e-3f, vzt::Tau);
        if (inputs.mouseLeftPressed)
            t = inputs.mousePosition.x * vzt::Tau / static_cast<float>(window.getWidth());

        const vzt::Quat rotation = glm::angleAxis(t, camera.up);

        float distanceScale = 1.f;
        if (inputs.mouseLeftPressed)
            distanceScale = inputs.mousePosition.y / static_cast<float>(window.getHeight());

        const float distance = distanceScale * 15.f * bbRadius / std::tan(camera.fov * .5f);

        const vzt::Vec3 cameraPosition  = target - camera.front * distance;
        const vzt::Vec3 currentPosition = rotation * (cameraPosition - target) + target;

        const vzt::Vec3 direction  = glm::normalize(target - currentPosition);
        const vzt::Vec3 reference  = camera.front;
        const float     projection = glm::dot(reference, direction);
        if (std::abs(projection) < 1.f - 1e-6f) // If direction and reference are not the same
            orientation = glm::rotation(reference, direction);
        else if (projection < 0.f) // If direction and reference are opposite
            orientation = glm::angleAxis(-vzt::Pi, camera.up);

        vzt::Mat4  view = camera.getViewMatrix(currentPosition, orientation);
        std::array matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};

        vzt::CommandBuffer commands = commandPool[submission->imageId];
        commands.begin();

        GenerationInput generationInput = {
            MaxInstanceCount,
            uint32_t(inputs.time),
        };

        generationUbo.write(commands, generationInput, submission->imageId);
        modelsUbo.write(commands, matrices, submission->imageId);

        commands.reset(queryPool, submission->imageId * (graph.size() + 1) * 2, (graph.size() + 1) * 2);
        commands.writeTimeStamp(queryPool, startId + graph.size() * 2 + 0, vzt::PipelineStage::BottomOfPipe);
        for (uint32_t i = 0; i < graph.size(); i++)
        {
            auto& pass = graph[i];
            commands.writeTimeStamp(queryPool, startId + i * 2 + 0, vzt::PipelineStage::BottomOfPipe);
            pass->record(submission->imageId, commands);
            commands.writeTimeStamp(queryPool, startId + i * 2 + 1, vzt::PipelineStage::BottomOfPipe);
        }
        commands.writeTimeStamp(queryPool, startId + graph.size() * 2 + 1, vzt::PipelineStage::BottomOfPipe);

        {
            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = swapchain.getImage(submission->imageId);
            imageBarrier.oldLayout = vzt::ImageLayout::ColorAttachmentOptimal;
            imageBarrier.newLayout = vzt::ImageLayout::PresentSrcKHR;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::BottomOfPipe, imageBarrier);
        }
        commands.end();

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
        {
            // Wait all commands execution
            device.wait();

            // Apply screen size update
            vzt::Extent2D extent = swapchain.getExtent();
            camera.aspectRatio   = static_cast<float>(extent.width) / static_cast<float>(extent.height);

            graph.resize(extent);
        }
    }
}
