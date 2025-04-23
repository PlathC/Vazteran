#include <glm/gtc/matrix_access.hpp>
#include <vzt/Core/Logger.hpp>
#include <vzt/Data/Camera.hpp>
#include <vzt/Data/Mesh.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/IOMesh.hpp>
#include <vzt/Utils/RenderGraph.hpp>
#include <vzt/Vulkan/Pipeline/ComputePipeline.hpp>
#include <vzt/Vulkan/QueryPool.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

template <class Type>
void updateUniformBuffer(vzt::CommandBuffer& commands, vzt::Buffer& buffer, std::size_t offset, Type* ptr,
                         std::size_t size = 1)
{
    assert(buffer.isMappable());

    uint8_t* data = buffer.map();
    std::memcpy(data + offset, ptr, size * sizeof(Type));
    buffer.unMap();

    vzt::BufferBarrier barrier{{buffer, buffer.size()}, vzt::Access::TransferWrite, vzt::Access::UniformRead};
    commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader, barrier);
}

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

    constexpr uint32_t MaxInstanceCount = 512;
    constexpr uint32_t WorkGroupSize    = 256;

    auto       window   = vzt::Window{ApplicationName, 1024, 1024};
    auto       instance = vzt::Instance{window};
    const auto surface  = vzt::Surface{window, instance};

    auto deviceBuilder = vzt::DeviceBuilder::standard();
    deviceBuilder.add(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    auto device = instance.getDevice(deviceBuilder, surface);

    const auto  queue    = device.getQueue(vzt::QueueType::Graphics);
    auto        hardware = device.getHardware();
    const float period   = hardware.getProperties().limits.timestampPeriod;

    auto swapchain = vzt::Swapchain{device, surface};
    auto compiler  = vzt::Compiler(instance);
    auto graph     = vzt::RenderGraph{swapchain};

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

    const auto vertexBuffer = vzt::Buffer::fromData<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer);
    const auto indexBuffer  = vzt::Buffer::fromData<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);

    vzt::VertexInputDescription vertexDescription{};
    vertexDescription.add(vzt::VertexBinding::Typed<VertexInput>(0));
    vertexDescription.add(vzt::VertexAttribute{0, 0, vzt::Format::R32G32B32SFloat, 0}); // Position
    vertexDescription.add(
        vzt::VertexAttribute{offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0}); // Normal

    auto instancesPosition = graph.addStorage(vzt::StorageBuilder{
        device,
        sizeof(vzt::Vec4f) * MaxInstanceCount,
        vzt::BufferUsage::StorageBuffer,
    });
    auto drawCommands      = graph.addStorage(vzt::StorageBuilder{
        device,
        sizeof(VkDrawIndexedIndirectCommand),
        vzt::BufferUsage::StorageBuffer | vzt::BufferUsage::IndirectBuffer,
        vzt::MemoryLocation::Device,
        true,
    });

    // Instance generation pass
    auto& instanceGeneration       = graph.addPass("InstanceGeneration", queue, vzt::PassType::Compute);
    auto& instanceGenerationLayout = instanceGeneration.getDescriptorLayout();
    instanceGenerationLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
    instanceGeneration.addStorageOutput(1, instancesPosition);
    instanceGeneration.addStorageOutput(2, drawCommands);

    auto computeInstanceProgram = vzt::Program(device);
    computeInstanceProgram.setShader(compiler("shaders/deferred/instance_generation.slang", "main"));

    auto computeInstancePipeline = vzt::ComputePipeline(device);
    computeInstancePipeline.setProgram(computeInstanceProgram);
    computeInstancePipeline.setDescriptorLayout(instanceGenerationLayout);
    computeInstancePipeline.compile();

    instanceGeneration.setRecordFunction<vzt::LambdaRecorder>(
        [&](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
            vzt::View<vzt::Buffer> buffer = graph.getStorage(i, drawCommands);

            uint8_t*                           data = buffer->map();
            const VkDrawIndexedIndirectCommand defaultCommand{uint32_t(mesh.indices.size()), 0, 0, 0, 0};
            std::memcpy(data, &defaultCommand, sizeof(VkDrawIndexedIndirectCommand));
            buffer->unMap();

            vzt::BufferBarrier barrier{{buffer, buffer->size()}, vzt::Access::TransferWrite, vzt::Access::ShaderWrite};
            commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader, barrier);

            commands.bind(computeInstancePipeline, set);
            commands.dispatch(static_cast<uint32_t>(std::ceil(MaxInstanceCount / static_cast<float>(WorkGroupSize))));
        });

    // Draw geometry pass
    auto position = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment, vzt::Format::R32G32B32A32SFloat});
    auto normal   = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment, vzt::Format::R8G8B8A8SNorm});
    auto depth    = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry       = graph.addPass("Geometry", queue);
    auto& geometryLayout = geometry.getDescriptorLayout();
    geometryLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
    geometry.addStorageInput(1, instancesPosition);
    geometry.addStorageInputIndirect(drawCommands);
    geometry.addColorOutput(position);
    geometry.addColorOutput(normal);
    geometry.setDepthOutput(depth);

    auto geometryProgram = vzt::Program(device);
    geometryProgram.setShader(compiler("shaders/deferred/triangle.slang", "vertexMain"));
    geometryProgram.setShader(compiler("shaders/deferred/triangle.slang", "fragmentMain"));

    auto geometryPipeline = vzt::GraphicPipeline(device);
    geometryPipeline.setViewport(vzt::Viewport{swapchain.getExtent()});
    geometryPipeline.setProgram(geometryProgram);
    geometryPipeline.setDescriptorLayout(geometryLayout);
    geometryPipeline.setVertexInputDescription(vertexDescription);

    auto& geometryRasterization    = geometryPipeline.getRasterization();
    geometryRasterization.cullMode = vzt::CullMode::None;

    geometry.setRecordFunction<vzt::LambdaRecorder>(
        [&](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
            const vzt::View<vzt::Buffer> buffer = graph.getStorage(i, drawCommands);

            commands.bind(geometryPipeline, set);
            commands.bindVertexBuffer(vertexBuffer);
            commands.bindIndexBuffer(indexBuffer, 0);
            commands.drawIndexedIndirect({buffer, buffer->size()}, 1, 0);
        });

    auto  composed      = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment});
    auto  composedDepth = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});
    auto& shading       = graph.addPass("Shading", queue);

    auto& shadingLayout = shading.getDescriptorLayout();
    shading.addColorInput(0, position);
    shading.addColorInput(1, normal);
    shading.addColorOutput(composed, "", vzt::Vec4(1.f, 0.91f, 0.69f, 1.f));
    shading.setDepthOutput(composedDepth);

    auto shadingProgram = vzt::Program(device);
    shadingProgram.setShader(compiler("shaders/deferred/deferred_blinn_phong.slang", "vertexMain"));
    shadingProgram.setShader(compiler("shaders/deferred/deferred_blinn_phong.slang", "fragmentMain"));

    auto shadingPipeline = vzt::GraphicPipeline(device);
    shadingPipeline.setProgram(shadingProgram);
    shadingPipeline.setViewport(vzt::Viewport{swapchain.getExtent()});
    shadingPipeline.setDescriptorLayout(shadingLayout);

    auto& shadingRasterization     = shadingPipeline.getRasterization();
    shadingRasterization.cullMode  = vzt::CullMode::Front;
    shadingRasterization.frontFace = vzt::FrontFace::CounterClockwise;

    shading.setRecordFunction<vzt::LambdaRecorder>(
        [&shadingPipeline](uint32_t, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
            commands.bind(shadingPipeline, set);
            commands.draw(3);
        });

    graph.setBackBuffer(composed);
    graph.compile();

    const vzt::QueryPool queryPool = {
        device,
        vzt::QueryType::Timestamp,
        swapchain.getImageNb() * (graph.size() + 1) * 2,
    };

    geometryPipeline.compile(geometry.getRenderPass());
    shadingPipeline.compile(shading.getRenderPass());

    // Initialize buffer with default values
    const std::size_t modelsAlignment = hardware.getUniformAlignment(sizeof(vzt::Mat4) * 3);
    vzt::Buffer       modelsUbo{device, modelsAlignment * swapchain.getImageNb(), vzt::BufferUsage::UniformBuffer,
                          vzt::MemoryLocation::Device, true};

    const std::size_t generationAlignment = hardware.getUniformAlignment(sizeof(GenerationInput));
    vzt::Buffer generationUbo{device, generationAlignment * swapchain.getImageNb(), vzt::BufferUsage::UniformBuffer,
                              vzt::MemoryLocation::Device, true};

    // Assign buffer parts to their respective image
    vzt::DescriptorPool& instanceGenerationDescriptorPool = instanceGeneration.getDescriptorPool();
    vzt::DescriptorPool& geometryDescriptorPool           = geometry.getDescriptorPool();

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        vzt::BufferSpan        generationSpan{generationUbo, sizeof(GenerationInput), i * generationAlignment};
        vzt::IndexedDescriptor ubos{};
        ubos[0] = vzt::DescriptorBuffer{vzt::DescriptorType::UniformBuffer, generationSpan};
        instanceGenerationDescriptorPool.update(i, ubos);

        vzt::BufferSpan modelSpan{modelsUbo, sizeof(vzt::Mat4) * 3u, i * modelsAlignment};
        ubos.clear();
        ubos[0] = vzt::DescriptorBuffer{vzt::DescriptorType::UniformBuffer, modelSpan};
        geometryDescriptorPool.update(i, ubos);
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

        vzt::Mat4                view = camera.getViewMatrix(currentPosition, orientation);
        std::array<vzt::Mat4, 3> matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};

        vzt::CommandBuffer commands = commandPool[submission->imageId];
        commands.begin();

        GenerationInput generationInput{
            MaxInstanceCount,
            uint32_t(inputs.time),
        };

        std::size_t offset = submission->imageId * generationAlignment;
        updateUniformBuffer(commands, generationUbo, offset, &generationInput);

        offset = submission->imageId * modelsAlignment;
        updateUniformBuffer(commands, modelsUbo, offset, matrices.data(), matrices.size());

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

        commands.end();

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
        {
            // Wait all commands execution
            device.wait();

            // Apply screen size update
            vzt::Extent2D extent = swapchain.getExtent();
            camera.aspectRatio   = static_cast<float>(extent.width) / static_cast<float>(extent.height);

            geometryPipeline.resize(vzt::Viewport{extent});
            shadingPipeline.resize(vzt::Viewport{extent});
            graph.resize(extent);
        }
    }
}
