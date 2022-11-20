#include <array>
#include <cassert>
#include <cstdlib>

#include <glm/mat4x4.hpp>
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
    descriptorLayout.addBinding(0, vzt::DescriptorType::UniformBuffer); // Model    { mat4 mvp;    mat4  normal }
    descriptorLayout.addBinding(1, vzt::DescriptorType::UniformBuffer); // Material { vec3 albedo; float shininess; }
    descriptorLayout.compile();
    pipeline.setDescriptorLayout(descriptorLayout);

    vzt::VertexInputDescription vertexDescription{};
    vertexDescription.add(vzt::VertexBinding{0, sizeof(float) * 8});
    vertexDescription.add(vzt::VertexAttribute{sizeof(float) * 0, 0, vzt::Format::R32G32B32SFloat, 0});
    vertexDescription.add(vzt::VertexAttribute{sizeof(float) * 3, 1, vzt::Format::R32G32SFloat, 0});
    vertexDescription.add(vzt::VertexAttribute{sizeof(float) * 5, 2, vzt::Format::R32G32B32SFloat, 0});
    pipeline.setVertexInputDescription(vertexDescription);

    std::vector<vzt::RenderPass> passes{};
    passes.reserve(swapchain.getImageNb());
    for (std::size_t i = 0; i < swapchain.getImageNb(); i++)
    {
        passes.emplace_back(device);

        vzt::RenderPass& pass = passes.back();

        // clang-format off
        pass.addColor(vzt::AttachmentUse{
            vzt::Format::R16G16B16A16SFloat,
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
        depth.clearValue = vzt::Vec4{1.f, 0.f, 0.f, 0.f};
        pass.setDepth(std::move(depth));
        pass.compile();
        // clang-format on
    }

    pipeline.compile(passes.back());

    vzt::DescriptorPool descriptorPool{device, descriptorLayout, swapchain.getImageNb()};
    descriptorPool.allocate(swapchain.getImageNb(), descriptorLayout);

    const std::size_t modelsAlignment = hardware.getUniformAlignment(sizeof(glm::mat4) * 3u);
    vzt::Buffer       modelsUbo       = vzt::Buffer(device, modelsAlignment * 1u, vzt::BufferUsage::UniformBuffer);

    const std::size_t materialsAlignment = hardware.getUniformAlignment<glm::vec4>();
    vzt::Buffer       materialsUbo = vzt::Buffer(device, materialsAlignment * 1u, vzt::BufferUsage::UniformBuffer);

    const vzt::Camera camera{};

    const vzt::Vec3 minimum{-10.f};
    const vzt::Vec3 maximum{10.f};
    const vzt::Vec3 target   = (minimum + maximum) * .5f;
    const float     bbRadius = glm::compMax(glm::abs(maximum - target));
    const float     distance = bbRadius / std::tan(camera.fov * .5f);

    const vzt::Vec3 direction = vzt::Camera::Front;
    const vzt::Vec3 position  = target + direction * 2.f * distance;

    const vzt::Mat4                view = camera.getViewMatrix(position, vzt::Quat{});
    const std::array<vzt::Mat4, 3> matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};
    modelsUbo.update<vzt::Mat4>(matrices);

    const vzt::Vec4 material{1.f, 1.f, 1.f, 16.f};
    materialsUbo.update<vzt::Vec4>(material);

    vzt::Indexed<vzt::BufferSpan> ubos{};
    ubos[0] = vzt::BufferSpan{modelsUbo, sizeof(glm::mat4) * 2u};
    ubos[1] = vzt::BufferSpan{materialsUbo, sizeof(glm::vec4)};
    descriptorPool.update(ubos);

    const vzt::Mesh mesh     = vzt::readObj("samples/Bunny/bunny.obj");
    vzt::Buffer vertexBuffer = vzt::Buffer::fromData<vzt::Vec3>(device, mesh.vertices, vzt::BufferUsage::VertexBuffer);
    vzt::Buffer indexBuffer  = vzt::Buffer::fromData<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);

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

            commands.bindVertexBuffer(vertexBuffer);
            for (const auto& subMesh : mesh.subMeshes)
                commands.drawIndexed(indexBuffer, subMesh.indices);

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
