#include <glm/gtc/matrix_access.hpp>
#include <vzt/Core/Logger.hpp>
#include <vzt/Data/Camera.hpp>
#include <vzt/Data/Mesh.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/IOMesh.hpp>
#include <vzt/Utils/RenderGraph.hpp>
#include <vzt/Vulkan/BufferType.hpp>
#include <vzt/Vulkan/Pipeline/ComputePipeline.hpp>
#include <vzt/Vulkan/QueryPool.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

struct alignas(16) GenerationInput
{
    uint32_t gridX, gridY, gridZ;
};

struct alignas(16) RaycastInput
{
    vzt::Mat4  worldToScreen;
    vzt::Vec4f cameraPos;
    vzt::Vec4f texelScale;
};

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Particles";

    constexpr uint32_t WorkGroupSize = 8;
    constexpr uint32_t GridWidth     = 8 * 8;
    constexpr uint32_t MipLevel      = 6;

    auto       window   = vzt::Window{ApplicationName, 1280, 720};
    auto       instance = vzt::Instance{window};
    const auto surface  = vzt::Surface{window, instance};

    auto deviceBuilder = vzt::DeviceBuilder::standard();
    auto device        = instance.getDevice(deviceBuilder, surface);

    auto        hardware = device.getHardware();
    const float period   = hardware.getProperties().limits.timestampPeriod;

    std::vector<uint32_t> indices = {
        0, 2, 1, 2, 3, 1, //
        5, 4, 1, 1, 4, 0, //
        0, 4, 6, 0, 6, 2, //
    };
    vzt::Buffer indexBuffer = vzt::Buffer::From<uint32_t>(device, indices, vzt::BufferUsage::IndexBuffer);

    auto swapchain = vzt::Swapchain{device, surface};
    auto compiler  = vzt::Compiler(instance);
    auto graph     = vzt::RenderGraph{device, swapchain};

    auto sdfTexture = graph.addAttachment(vzt::AttachmentBuilder{
        .size      = vzt::Extent3D{GridWidth, GridWidth, GridWidth},
        .usage     = vzt::ImageUsage::TransferDst | vzt::ImageUsage::Storage | vzt::ImageUsage::Sampled,
        .format    = vzt::Format::R16UNorm,
        .type      = vzt::ImageType::T3D,
        .mipLevels = MipLevel,
    });

    // Instance generation pass
    auto& sdfGeneration = graph.addCompute("SDF generation", compiler("shaders/sdf/grid.slang", "main"));
    {
        sdfGeneration.addAttachmentInputOutput(1, sdfTexture);
        sdfGeneration.setRecordFunction<vzt::LambdaRecorder>( //
            [&](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
                commands.bind(sdfGeneration.getPipeline(), set);
                commands.dispatch(GridWidth / WorkGroupSize, GridWidth / WorkGroupSize, GridWidth / WorkGroupSize);
            });
    }

    // Draw geometry pass
    auto color = graph.addAttachment({vzt::ImageUsage::ColorAttachment});
    auto depth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry = graph.addGraphics("Geometry", compiler("shaders/sdf/raycast.slang"));
    {
        geometry.addStorageInput(1, sdfTexture);
        geometry.addColorOutput(color, "", vzt::Vec4(1.f, 0.91f, 0.69f, 1.f));
        geometry.setDepthOutput(depth);

        auto& geometryPipeline = geometry.getPipeline();
        geometryPipeline.setViewport(vzt::Viewport{swapchain.getExtent()});

        auto& geometryRasterization    = geometryPipeline.getRasterization();
        geometryRasterization.cullMode = vzt::CullMode::None;

        geometry.setRecordFunction<vzt::LambdaRecorder>(
            [&](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
                commands.bind(geometryPipeline, set);
                commands.drawIndexed(indexBuffer, {0, 3 * 3 * 2});
            });
    }

    graph.setBackBuffer(color);
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
    vzt::DescriptorPool& generationDescriptorPool = sdfGeneration.getDescriptorPool();
    vzt::DescriptorPool& geometryDescriptorPool   = geometry.getDescriptorPool();

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        generationDescriptorPool.update(i, {{0, generationUbo.getDescriptor(i)}});
        geometryDescriptorPool.update(i, {{0, modelsUbo.getDescriptor(i)}});
    }

    // Compute AABB to place camera in front of the model
    vzt::Vec3 minimum{-1.f, -1.f, -1.f};
    vzt::Vec3 maximum{1.f, 1.f, 1.f};

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

        const float distance = distanceScale * 13.f * bbRadius / std::tan(camera.fov * .5f);

        const vzt::Vec3 cameraPosition  = target - camera.front * distance;
        const vzt::Vec3 currentPosition = rotation * (cameraPosition - target) + target;

        const vzt::Vec3 direction  = glm::normalize(target - currentPosition);
        const vzt::Vec3 reference  = camera.front;
        const float     projection = glm::dot(reference, direction);
        if (std::abs(projection) < 1.f - 1e-6f) // If direction and reference are not the same
            orientation = glm::rotation(reference, direction);
        else if (projection < 0.f) // If direction and reference are opposite
            orientation = glm::angleAxis(-vzt::Pi, camera.up);

        vzt::Mat4    view         = camera.getViewMatrix(currentPosition, orientation);
        RaycastInput raycastInput = {
            .worldToScreen = glm::transpose(camera.getProjectionMatrix() * view),
        };
        std::array matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};

        vzt::CommandBuffer commands = commandPool[submission->imageId];
        commands.begin();

        GenerationInput generationInput = {.gridX = GridWidth, .gridY = GridWidth, .gridZ = GridWidth};

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
