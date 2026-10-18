#include <vzt/camera.hpp>
#include <vzt/compiler.hpp>
#include <vzt/core/logger.hpp>
#include <vzt/render_graph.hpp>
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
    const std::string ApplicationName = "Vazteran Particles";

    constexpr uint32_t MaxInstanceCount = 2 << 19;
    constexpr uint32_t WorkGroupSize    = 256;

    auto       window   = vzt::Window{ApplicationName, 1500, 700};
    auto       instance = vzt::Instance{ApplicationName, window.getConfiguration()};
    const auto surface  = vzt::Surface{window, instance};

    auto device = instance.getDevice(vzt::DeviceBuilder::standard(), surface);

    auto swapchain = vzt::Swapchain{device, surface};
    auto compiler  = vzt::Compiler(instance);
    auto graph     = vzt::RenderGraph{device};

    auto instancesPosition = graph.addStorage( //
        vzt::StorageBuilder{sizeof(vzt::Vec4f) * MaxInstanceCount, vzt::BufferUsage::StorageBuffer});
    auto drawCommands      = graph.addStorage(vzt::StorageBuilder::fromType<VkDrawIndexedIndirectCommand>(
        vzt::BufferUsage::StorageBuffer | vzt::BufferUsage::IndirectBuffer, vzt::MemoryLocation::Device, true, true));

    // Instance generation pass
    auto& instanceGeneration = graph.addCompute( //
        "InstanceGeneration", compiler("shaders/particles/instance_generation.slang", "main"));
    {
        instanceGeneration.write(instancesPosition, vzt::Use::StorageBuffer, "instances");
        instanceGeneration.write(drawCommands, vzt::Use::StorageBuffer, "draw");

        instanceGeneration.setRecordFunction([&](vzt::ComputeContext& ctx) {
            vzt::View<vzt::Buffer> buffer = ctx.buffer(drawCommands);

            uint8_t*                           data           = buffer->map();
            const VkDrawIndexedIndirectCommand defaultCommand = {6, 0, 0, 0, 0};
            std::memcpy(data, &defaultCommand, sizeof(VkDrawIndexedIndirectCommand));
            buffer->unMap();

            vzt::BufferBarrier barrier{*buffer, vzt::Access::TransferWrite, vzt::Access::ShaderWrite};
            ctx.commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader, barrier);
            ctx.commands.dispatch(
                static_cast<uint32_t>(std::ceil(MaxInstanceCount / static_cast<float>(WorkGroupSize))));
        });
    }

    // Draw geometry pass
    auto color = graph.addAttachment({vzt::ImageUsage::ColorAttachment});
    auto depth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry = graph.addGraphics("Geometry", compiler("shaders/particles/circle.slang"));
    {
        geometry.read(instancesPosition, vzt::Use::StorageBuffer, "instances");
        geometry.read(drawCommands, vzt::Use::Indirect);
        geometry.color(color, {.clear = vzt::Vec4(1.f, 0.91f, 0.69f, 1.f),
                               .blend = {.blendEnable = true, .colorBlendOp = vzt::BlendOp::Add}});
        geometry.depth(depth);

        auto& builder = geometry.getBuilder();
        builder.set(vzt::Rasterization{.cullMode = vzt::CullMode::None});

        geometry.setRecordFunction([&](vzt::GraphicsContext& ctx) {
            const vzt::View<vzt::Buffer> buffer = ctx.buffer(drawCommands);
            ctx.commands.drawIndirect(*buffer, 1, 0);
        });
    }

    // Initialize buffer with default values
    const uint32_t     frameNb       = swapchain.getFrameNb();
    vzt::UniformBuffer modelsUbo     = vzt::UniformBuffer(device, sizeof(vzt::Mat4) * 3, frameNb, true);
    vzt::UniformBuffer generationUbo = vzt::UniformBuffer(device, sizeof(GenerationInput), frameNb, true);
    instanceGeneration.bind(generationUbo, "uniforms");
    geometry.bind(modelsUbo, "Model");

    graph.setBackbuffer(swapchain, color);
    graph.setProfiling(true);
    graph.compile();

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getFrameNb());

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

        if (hasBenchmark)
            for (uint32_t i = 0; i < graph.size(); i++)
                vzt::logger::info("{}: {}ms", graph[i]->getName(), graph[i]->getDuration(submission->frameId));
        hasBenchmark |= submission->frameId == (swapchain.getFrameNb() - 1);

        // Per frame update
        vzt::Quat orientation = {1.f, 0.f, 0.f, 0.f};

        float t = std::fmod(static_cast<float>(inputs.time) * 1e-3f, vzt::Tau);
        if (inputs.mouseLeftPressed)
            t = inputs.mousePosition.x * vzt::Tau / static_cast<float>(window.getWidth());

        const vzt::Quat rotation = glm::angleAxis(t, camera.right);

        float distanceScale = 1.f;
        if (inputs.mouseLeftPressed)
            distanceScale = inputs.mousePosition.y / static_cast<float>(window.getHeight());

        camera.fov           = glm::radians(20.f);
        const float distance = distanceScale * 5.f * bbRadius / std::tan(camera.fov * .5f);

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

        vzt::CommandBuffer commands = commandPool[submission->frameId];
        commands.begin();

        GenerationInput generationInput = {
            MaxInstanceCount,
            uint32_t(inputs.time),
        };

        generationUbo.write(commands, generationInput, submission->frameId);
        modelsUbo.write(commands, matrices, submission->frameId);

        graph.setBackbufferImage(submission->frameId, submission->imageId);
        graph.record(submission->frameId, commands);

        commands.end();

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
        {
            // Wait all commands execution
            device.wait();

            // Apply screen size update
            vzt::Extent2D extent = swapchain.getExtent();
            camera.aspectRatio   = static_cast<float>(extent.width) / static_cast<float>(extent.height);

            graph.setBackbuffer(swapchain, color); // New swapchain images
            graph.resize(extent);
        }
    }
}
