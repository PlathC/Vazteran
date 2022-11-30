#include <array>
#include <cassert>
#include <cstdlib>

#include <vzt/Core/File.hpp>
#include <vzt/Data/Camera.hpp>
#include <vzt/Data/Mesh.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/MeshLoader.hpp>
#include <vzt/Vulkan/Attachment.hpp>
#include <vzt/Vulkan/Command.hpp>
#include <vzt/Vulkan/Descriptor.hpp>
#include <vzt/Vulkan/GraphicPipeline.hpp>
#include <vzt/Vulkan/RenderPass.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

#include "vzt/Vulkan/FrameBuffer.hpp"

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Blank";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder{}, surface);
    auto hardware  = device.getHardware();
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    auto program  = vzt::Program(device);
    auto compiler = vzt::Compiler();
    program.setShader(compiler.compile("shaders/triangle.vert", vzt::ShaderStage::Vertex));
    program.setShader(compiler.compile("shaders/triangle.frag", vzt::ShaderStage::Fragment));

    auto pipeline = vzt::GraphicPipeline(device, program, vzt::Viewport{window.getExtent()});

    vzt::DescriptorLayout descriptorLayout{device};

    descriptorLayout.addBinding(0, vzt::DescriptorType::UniformBuffer); // Model {
                                                                        //     mat4 modelViewMatrix;
                                                                        //     mat4 projectionMatrix;
                                                                        //     mat4 normalMatrix;
                                                                        // }
    descriptorLayout.addBinding(1, vzt::DescriptorType::UniformBuffer); // Material { vec3 albedo; float shininess; }
    descriptorLayout.compile();
    pipeline.setDescriptorLayout(descriptorLayout);

    vzt::VertexInputDescription vertexDescription{};
    vertexDescription.add(vzt::VertexBinding{0, sizeof(float) * 8});
    vertexDescription.add(vzt::VertexAttribute{sizeof(float) * 0, 0, vzt::Format::R32G32B32SFloat, 0});
    vertexDescription.add(vzt::VertexAttribute{sizeof(float) * 3, 1, vzt::Format::R32G32B32SFloat, 0});
    pipeline.setVertexInputDescription(vertexDescription);

    vzt::RenderPass renderPass{device};

    // clang-format off
    renderPass.addColor(vzt::AttachmentUse{
        vzt::Format::B8G8R8A8SRGB,
        vzt::ImageLayout::Undefined,
        vzt::ImageLayout::ColorAttachmentOptimal,
        vzt::ImageLayout::ColorAttachmentOptimal
    });
    // clang-format on

    // clang-format off
    vzt::AttachmentUse depth{
        hardware.getDepthFormat(),
        vzt::ImageLayout::Undefined,
        vzt::ImageLayout::DepthStencilAttachmentOptimal,
        vzt::ImageLayout::DepthStencilAttachmentOptimal
    };
    // clang-format on

    depth.clearValue = vzt::Vec4{1.f, 0.f, 0.f, 0.f};
    renderPass.setDepth(std::move(depth));
    renderPass.compile();

    pipeline.compile(renderPass);

    const vzt::Format depthFormat = hardware.getDepthFormat();

    std::vector<vzt::Image>       depthStencils;
    std::vector<vzt::FrameBuffer> frameBuffers;
    depthStencils.reserve(swapchain.getImageNb());
    frameBuffers.reserve(swapchain.getImageNb());
    for (std::size_t i = 0; i < swapchain.getImageNb(); i++)
    {
        depthStencils.emplace_back(device, window.getExtent(), vzt::ImageUsage::DepthStencilAttachment, depthFormat);

        const auto& image = swapchain.getImage(i);
        frameBuffers.emplace_back(device, window.getExtent());

        vzt::FrameBuffer& frameBuffer = frameBuffers.back();
        frameBuffer.addAttachment(vzt::ImageView(device, image, vzt::ImageAspect::Color));
        frameBuffer.addAttachment(vzt::ImageView(device, depthStencils.back(), vzt::ImageAspect::Depth));
        frameBuffer.compile(renderPass);
    }

    vzt::DescriptorPool descriptorPool{device, descriptorLayout, swapchain.getImageNb()};
    descriptorPool.allocate(swapchain.getImageNb(), descriptorLayout);

    vzt::Mesh mesh = vzt::readObj("samples/Bunny/Bunny.obj");
    vzt::Vec3 minimum{std::numeric_limits<float>::max()};
    vzt::Vec3 maximum{std::numeric_limits<float>::lowest()};
    for (vzt::Vec3& vertex : mesh.vertices)
    {
        minimum = glm::min(minimum, vertex);
        maximum = glm::max(maximum, vertex);
    }

    vzt::Camera camera{};
    camera.front = vzt::Vec3(0.f, 1.f, 0.f);
    camera.up    = vzt::Vec3(0.f, 0.f, 1.f);
    camera.right = vzt::Vec3(1.f, 0.f, 0.f);

    const vzt::Vec3 target   = (minimum + maximum) * .5f;
    const float     bbRadius = glm::compMax(glm::abs(maximum - target));
    const float     distance = bbRadius / std::tan(camera.fov * .5f);

    const vzt::Vec3 position = target - camera.front * 2.f * distance;

    const std::size_t modelsAlignment = hardware.getUniformAlignment(sizeof(glm::mat4) * 3u);
    vzt::Buffer       modelsUbo       = vzt::Buffer(device, modelsAlignment * 1u, vzt::BufferUsage::UniformBuffer);
    const vzt::Mat4   view            = camera.getViewMatrix(position, vzt::Quat{});
    const std::array<vzt::Mat4, 3> matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};
    modelsUbo.update<vzt::Mat4>(matrices);

    const std::size_t materialsAlignment = hardware.getUniformAlignment<glm::vec4>();
    vzt::Buffer       materialsUbo = vzt::Buffer(device, materialsAlignment * 1u, vzt::BufferUsage::UniformBuffer);
    const vzt::Vec4   material{1.f, 1.f, 1.f, 16.f};
    materialsUbo.update<vzt::Vec4>(material);

    vzt::Indexed<vzt::BufferSpan> ubos{};
    ubos[0] = vzt::BufferSpan{modelsUbo, sizeof(glm::mat4) * 3u};
    ubos[1] = vzt::BufferSpan{materialsUbo, sizeof(glm::vec4)};
    descriptorPool.update(ubos);

    struct alignas(16) VertexInput
    {
        vzt::Vec3 inPosition;
        vzt::Vec3 inNormal;
    };

    std::vector<VertexInput> inputs;
    inputs.reserve(mesh.vertices.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
        inputs.emplace_back(VertexInput{mesh.vertices[i], mesh.normals[i]});

    const auto vertexBuffer = vzt::Buffer::fromData<VertexInput>(device, inputs, vzt::BufferUsage::VertexBuffer);
    const auto indexBuffer  = vzt::Buffer::fromData<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    while (window.update())
    {
        const auto& inputs = window.getInputs();
        if (inputs.windowResized)
            swapchain.setExtent(inputs.windowSize);

        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        const auto&        image    = swapchain.getImage(submission->imageId);
        vzt::CommandBuffer commands = commandPool[submission->imageId];
        {
            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::Undefined;
            imageBarrier.newLayout = vzt::ImageLayout::TransferDstOptimal;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.clear(image, vzt::ImageLayout::TransferDstOptimal, vzt::Vec4{0.96f, 0.64f, 0.64f, 1.f});

            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::TransferDstOptimal;
            imageBarrier.newLayout = vzt::ImageLayout::ColorAttachmentOptimal;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.begin(renderPass, frameBuffers[submission->imageId]);

            commands.bind(pipeline, descriptorPool[submission->imageId]);
            commands.bindVertexBuffer(vertexBuffer);
            for (const auto& subMesh : mesh.subMeshes)
                commands.drawIndexed(indexBuffer, subMesh.indices);

            commands.end();

            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::ColorAttachmentOptimal;
            imageBarrier.newLayout = vzt::ImageLayout::PresentSrcKHR;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);
        }

        graphicsQueue->submit(commands, *submission);
        if (swapchain.present())
            device.wait();
    }

    return EXIT_SUCCESS;
}
