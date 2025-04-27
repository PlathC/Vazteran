#include <array>
#include <cstdlib>

#include <vzt/Core/Logger.hpp>
#include <vzt/Data/Camera.hpp>
#include <vzt/Data/Mesh.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/IOMesh.hpp>
#include <vzt/Vulkan/BufferType.hpp>
#include <vzt/Vulkan/Command.hpp>
#include <vzt/Vulkan/Descriptor.hpp>
#include <vzt/Vulkan/FrameBuffer.hpp>
#include <vzt/Vulkan/Pipeline/GraphicsPipeline.hpp>
#include <vzt/Vulkan/RenderPass.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

struct VertexInput
{
    vzt::Vec3 inPosition;
    vzt::Vec3 inNormal;
};

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Base";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto compiler  = vzt::Compiler(instance);
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder::standard(), surface);
    auto hardware  = device.getHardware();
    auto swapchain = vzt::Swapchain{device, surface};

    vzt::RenderPass renderPass{device};
    {
        renderPass.addColor(vzt::AttachmentUse{
            .format        = vzt::Format::B8G8R8A8SRGB,
            .initialLayout = vzt::ImageLayout::Undefined,
            .finalLayout   = vzt::ImageLayout::ColorAttachmentOptimal,
            .usedLayout    = vzt::ImageLayout::ColorAttachmentOptimal,
            .clearValue    = vzt::Vec4(1.f, 0.91f, 0.69f, 1.f),
        });

        renderPass.setDepth(vzt::AttachmentUse{
            .format        = hardware.getDepthFormat(),
            .initialLayout = vzt::ImageLayout::Undefined,
            .finalLayout   = vzt::ImageLayout::DepthStencilAttachmentOptimal,
            .usedLayout    = vzt::ImageLayout::DepthStencilAttachmentOptimal,
            .clearValue    = vzt::Vec4{1.f, 0.f, 0.f, 0.f},
        });
    }
    renderPass.compile();

    const auto program  = vzt::Program(device, compiler("shaders/base/base.slang"));
    auto       pipeline = vzt::GraphicPipeline(program);
    {
        pipeline.setProgram(program);
        pipeline.setViewport(vzt::Viewport{swapchain.getExtent()});

        vzt::VertexInputDescription vertexDescription{};
        vertexDescription.add(vzt::VertexBinding::Typed<VertexInput>(0));
        vertexDescription.add(offsetof(VertexInput, inPosition), 0, vzt::Format::R32G32B32SFloat, 0);
        vertexDescription.add(offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0);
        pipeline.setVertexInputDescription(vertexDescription);
    }
    pipeline.compile(renderPass);

    // Initialize descriptors
    vzt::DescriptorPool descriptorPool{device, pipeline, swapchain.getImageNb()};
    vzt::UniformBuffer  ubo = {device, sizeof(vzt::Mat4) * 3, swapchain.getImageNb(), true};

    const vzt::Format depthFormat = hardware.getDepthFormat();
    vzt::Extent2D     extent      = swapchain.getExtent();

    std::vector<vzt::DeviceImage> depthStencils;
    std::vector<vzt::FrameBuffer> frameBuffers;
    depthStencils.reserve(swapchain.getImageNb());
    frameBuffers.reserve(swapchain.getImageNb());
    const auto createRenderObject = [&](uint32_t i) {
        const auto& image = swapchain.getImage(i);
        frameBuffers.emplace_back(device, extent);
        depthStencils.emplace_back(device, extent, vzt::ImageUsage::DepthStencilAttachment, depthFormat);

        vzt::FrameBuffer& frameBuffer = frameBuffers.back();
        frameBuffer.addAttachment(vzt::ImageView(device, image, vzt::ImageAspect::Color));
        frameBuffer.addAttachment(vzt::ImageView(device, depthStencils.back(), vzt::ImageAspect::Depth));
        frameBuffer.compile(renderPass);
    };

    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        descriptorPool.update(i, {{0, ubo.getDescriptor(i)}});
        createRenderObject(i);
    }

    // Vertex inputs
    vzt::Mesh   mesh = vzt::readObj("samples/Dragon/dragon.obj");
    vzt::Buffer vertexBuffer;
    vzt::Buffer indexBuffer;
    {
        std::vector<VertexInput> vertexInputs = std::vector<VertexInput>(mesh.vertices.size());
        for (std::size_t i = 0; i < mesh.vertices.size(); i++)
            vertexInputs[i] = VertexInput(mesh.vertices[i], mesh.normals[i]);

        vertexBuffer = vzt::Buffer::fromData<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer);
        indexBuffer  = vzt::Buffer::fromData<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);
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
    camera.up          = vzt::Vec3(0.f, 0.f, 1.f);
    camera.front       = vzt::Vec3(0.f, 1.f, 0.f);
    camera.right       = vzt::Vec3(1.f, 0.f, 0.f);
    camera.aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    const vzt::Vec3 target   = (minimum + maximum) * .5f;
    const float     bbRadius = glm::compMax(glm::abs(maximum - target));
    const float     distance = bbRadius / std::tan(camera.fov * .5f);
    const vzt::Vec3 position = target - camera.front * 1.15f * distance;

    // Actual rendering
    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    while (window.update())
    {
        const auto& inputs = window.getInputs();
        if (inputs.windowResized)
            swapchain.recreate();

        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        const uint32_t frame = submission->imageId;

        // Per frame update
        vzt::Quat orientation = {1.f, 0.f, 0.f, 0.f};

        float t = std::fmod(static_cast<float>(inputs.time) * 1e-3f, vzt::Tau);
        if (inputs.mouseLeftPressed)
            t = inputs.mousePosition.x * vzt::Tau / static_cast<float>(window.getWidth());

        const vzt::Quat rotation        = glm::angleAxis(t, camera.up);
        const vzt::Vec3 currentPosition = rotation * (position - target) + target;

        vzt::Vec3       direction  = glm::normalize(target - currentPosition);
        const vzt::Vec3 reference  = camera.front;
        const float     projection = glm::dot(reference, direction);
        if (std::abs(projection) < 1.f - 1e-6f) // If direction and reference are not the same
            orientation = glm::rotation(reference, direction);
        else if (projection < 0.f) // If direction and reference are opposite
            orientation = glm::angleAxis(-vzt::Pi, camera.up);

        vzt::Mat4                view = camera.getViewMatrix(currentPosition, orientation);
        std::array<vzt::Mat4, 3> matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};

        vzt::CommandBuffer commands = commandPool[frame];
        commands.begin();
        {
            ubo.write(commands, vzt::CSpan<vzt::Mat4>{matrices.data(), matrices.size()}, frame);
            commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader,
                             {ubo.getSpan(frame), vzt::Access::TransferWrite, vzt::Access::UniformRead});

            commands.beginPass(renderPass, frameBuffers[frame]);

            commands.bind(pipeline, descriptorPool[frame]);
            commands.bindVertexBuffer(vertexBuffer);
            for (const auto& subMesh : mesh.subMeshes)
                commands.drawIndexed(indexBuffer, subMesh.indices);

            commands.endPass();

            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = swapchain.getImage(submission->imageId);
            imageBarrier.oldLayout = vzt::ImageLayout::ColorAttachmentOptimal;
            imageBarrier.newLayout = vzt::ImageLayout::PresentSrcKHR;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);
        }
        commands.end();

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
        {
            // Wait all commands execution
            device.wait();

            // Apply screen size update
            extent             = swapchain.getExtent();
            camera.aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

            pipeline.resize(vzt::Viewport{extent});

            depthStencils.clear();
            frameBuffers.clear();
            for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
                createRenderObject(i);
        }
    }

    return EXIT_SUCCESS;
}
