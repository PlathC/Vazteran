#include <array>
#include <cstdlib>

#include <vzt/camera.hpp>
#include <vzt/compiler.hpp>
#include <vzt/core/logger.hpp>
#include <vzt/vulkan/command.hpp>
#include <vzt/vulkan/descriptor.hpp>
#include <vzt/vulkan/pipeline/graphics.hpp>
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

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Base";

    auto window   = vzt::Window{ApplicationName};
    auto instance = vzt::Instance{ApplicationName, window.getConfiguration()};

    auto compiler  = vzt::Compiler(instance);
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder::standard(), surface);
    auto hardware  = device.getHardware();
    auto swapchain = vzt::Swapchain{device, surface};

    const vzt::Format depthFormat = hardware.getDepthFormat();
    const auto        program     = vzt::Program(device, compiler("shaders/base/base.slang"));

    vzt::VertexInputDescription vertexDescription{};
    vertexDescription.add(vzt::VertexBinding::Typed<VertexInput>(0));
    vertexDescription.add(offsetof(VertexInput, inPosition), 0, vzt::Format::R32G32B32SFloat, 0);
    vertexDescription.add(offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0);

    const auto pipeline = vzt::GraphicsPipeline(vzt::GraphicsPipelineBuilder{program}
                                                    .set(vertexDescription)
                                                    .addColor(vzt::Format::B8G8R8A8SRGB)
                                                    .setDepth(depthFormat));

    // Initialize descriptors
    vzt::DescriptorPool descriptorPool{device, pipeline, swapchain.getImageNb()};
    vzt::UniformBuffer  ubo = {device, sizeof(vzt::Mat4) * 3, swapchain.getImageNb(), true};

    vzt::Extent2D extent = swapchain.getExtent();

    std::vector<vzt::ImageView>   imageViews    = std::vector<vzt::ImageView>{swapchain.getImageNb()};
    std::vector<vzt::ImageView>   depthViews    = std::vector<vzt::ImageView>{swapchain.getImageNb()};
    std::vector<vzt::DeviceImage> depthStencils = std::vector<vzt::DeviceImage>(swapchain.getImageNb());

    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        descriptorPool.update(i, {{0, ubo.getDescriptor(i)}});
        depthStencils[i] = vzt::DeviceImage(device, extent, vzt::ImageUsage::DepthStencilAttachment, depthFormat);
        imageViews[i]    = vzt::ImageView(device, swapchain.getImage(i), vzt::ImageAspect::Color);
        depthViews[i]    = vzt::ImageView(device, depthStencils[i], vzt::ImageAspect::Depth);
    }

    // Vertex inputs
    vzt::Mesh   mesh = vzt::readObj("samples/Dragon/dragon.obj");
    vzt::Buffer vertexBuffer;
    vzt::Buffer indexBuffer;
    {
        std::vector<VertexInput> vertexInputs = std::vector<VertexInput>(mesh.vertices.size());
        for (std::size_t i = 0; i < mesh.vertices.size(); i++)
            vertexInputs[i] = VertexInput(mesh.vertices[i], mesh.normals[i]);

        vertexBuffer = vzt::Buffer::From<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer);
        indexBuffer  = vzt::Buffer::From<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);
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

        vzt::Mat4  view = camera.getViewMatrix(currentPosition, orientation);
        std::array matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};

        extent = swapchain.getExtent();

        vzt::CommandBuffer commands = commandPool[frame];
        commands.begin();
        {
            ubo.write(commands, vzt::CSpan<vzt::Mat4>{matrices.data(), matrices.size()}, frame);
            commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader,
                             {ubo.getSpan(frame), vzt::Access::TransferWrite, vzt::Access::UniformRead});

            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = swapchain.getImage(submission->imageId);
            imageBarrier.oldLayout = vzt::ImageLayout::Undefined;
            imageBarrier.newLayout = vzt::ImageLayout::ColorAttachmentOptimal;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::VertexShader, imageBarrier);

            commands.setViewport(vzt::Viewport{.size = {extent.width, extent.height}});
            commands.setScissor(vzt::Scissor{.extent = extent});

            commands.beginRendering({
                .renderArea       = {0, 0, extent.width, extent.height},
                .colorAttachments = {{
                    .view       = imageViews[frame],
                    .layout     = vzt::ImageLayout::ColorAttachmentOptimal,
                    .clearValue = vzt::Vec4(1.f, 0.91f, 0.69f, 1.f),
                }},
                .depthAttachment =
                    vzt::RenderingInfo::RenderingAttachment{
                        .view       = depthViews[frame],
                        .layout     = vzt::ImageLayout::DepthStencilAttachmentOptimal,
                        .clearValue = vzt::Vec4(1.f, 0.f, 0.f, 0.f),
                    },
            });

            commands.bind(pipeline, descriptorPool[frame]);
            commands.bindVertexBuffer(vertexBuffer);
            for (const auto& subMesh : mesh.subMeshes)
                commands.drawIndexed(indexBuffer, subMesh.indices);

            commands.endRendering();

            imageBarrier           = vzt::ImageBarrier{};
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

            for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
            {
                descriptorPool.update(i, {{0, ubo.getDescriptor(i)}});
                depthStencils[i] =
                    vzt::DeviceImage(device, extent, vzt::ImageUsage::DepthStencilAttachment, depthFormat);
                imageViews[i] = vzt::ImageView(device, swapchain.getImage(i), vzt::ImageAspect::Color);
                depthViews[i] = vzt::ImageView(device, depthStencils[i], vzt::ImageAspect::Depth);
            }
        }
    }

    return EXIT_SUCCESS;
}
