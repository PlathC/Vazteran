#include <array>
#include <cassert>
#include <cstdlib>

#include <vzt/Core/Logger.hpp>
#include <vzt/Data/Camera.hpp>
#include <vzt/Data/Mesh.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/MeshLoader.hpp>
#include <vzt/Vulkan/Command.hpp>
#include <vzt/Vulkan/Descriptor.hpp>
#include <vzt/Vulkan/FrameBuffer.hpp>
#include <vzt/Vulkan/GraphicPipeline.hpp>
#include <vzt/Vulkan/RenderPass.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Base";

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

    auto pipeline = vzt::GraphicPipeline(device, program, vzt::Viewport{window.getExtent()});

    vzt::DescriptorLayout descriptorLayout{device};
    // Model { mat4 modelView; mat4 projection; mat4 normal; }
    descriptorLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
    // Material { vec3 albedo; float shininess; }
    descriptorLayout.addBinding(1, vzt::DescriptorType::UniformBuffer);
    descriptorLayout.compile();

    pipeline.setDescriptorLayout(descriptorLayout);

    vzt::VertexInputDescription vertexDescription{};
    struct alignas(16) VertexInput
    {
        vzt::Vec3 inPosition;
        vzt::Vec3 inNormal;
    };
    vertexDescription.add(vzt::VertexBinding::Typed<VertexInput>(0));
    vertexDescription.add(vzt::VertexAttribute{0, 0, vzt::Format::R32G32B32SFloat, 0}); //
    vertexDescription.add(vzt::VertexAttribute{offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0});
    pipeline.setVertexInputDescription(vertexDescription);

    pipeline.compile(renderPass);

    // Initialize buffer with default values
    const std::size_t modelsAlignment    = hardware.getUniformAlignment(sizeof(vzt::Mat4) * 3);
    const std::size_t materialsAlignment = hardware.getUniformAlignment<vzt::Vec4>();
    const std::size_t uniformByteNb      = modelsAlignment + materialsAlignment;
    vzt::Buffer       modelsUbo{device, uniformByteNb * swapchain.getImageNb(), vzt::BufferUsage::UniformBuffer};

    // Assign buffer parts to their respective image
    vzt::DescriptorPool descriptorPool{device, descriptorLayout, swapchain.getImageNb()};
    descriptorPool.allocate(swapchain.getImageNb(), descriptorLayout);

    // Vertex inputs
    vzt::Mesh mesh = vzt::readObj("samples/Dragon/dragon.obj");

    std::vector<VertexInput> vertexInputs;
    vertexInputs.reserve(mesh.vertices.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
        vertexInputs.emplace_back(VertexInput{mesh.vertices[i], mesh.normals[i]});

    const auto vertexBuffer = vzt::Buffer::fromData<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer);
    const auto indexBuffer  = vzt::Buffer::fromData<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);

    vzt::Extent2D                 extent = window.getExtent();
    std::vector<vzt::Image>       depthStencils;
    std::vector<vzt::FrameBuffer> frameBuffers;
    depthStencils.reserve(swapchain.getImageNb());
    frameBuffers.reserve(swapchain.getImageNb());

    const vzt::Format depthFormat = hardware.getDepthFormat();

    auto       graphicsQueue      = device.getQueue(vzt::QueueType::Graphics);
    auto       commandPool        = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    const auto createRenderObject = [&](uint32_t i) {
        // Render targets
        depthStencils.emplace_back(device, window.getExtent(), vzt::ImageUsage::DepthStencilAttachment, depthFormat);

        const auto& image = swapchain.getImage(i);
        frameBuffers.emplace_back(device, extent);

        vzt::FrameBuffer& frameBuffer = frameBuffers.back();
        frameBuffer.addAttachment(vzt::ImageView(device, image, vzt::ImageAspect::Color));
        frameBuffer.addAttachment(vzt::ImageView(device, depthStencils.back(), vzt::ImageAspect::Depth));
        frameBuffer.compile(renderPass);

        // Pre-record commands since they will not change during rendering
        vzt::CommandBuffer commands = commandPool[i];
        commands.begin();
        {
            commands.beginPass(renderPass, frameBuffers[i]);

            commands.bind(pipeline, descriptorPool[i]);
            commands.bindVertexBuffer(vertexBuffer);
            for (const auto& subMesh : mesh.subMeshes)
                commands.drawIndexed(indexBuffer, subMesh.indices);

            commands.endPass();

            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::ColorAttachmentOptimal;
            imageBarrier.newLayout = vzt::ImageLayout::PresentSrcKHR;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);
        }
        commands.end();
    };

    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        modelsUbo.update<vzt::Vec4>(vzt::Vec4{1.f}, i * uniformByteNb + modelsAlignment);

        vzt::BufferSpan modelSpan{modelsUbo, sizeof(vzt::Mat4) * 3u, i * uniformByteNb};
        vzt::BufferSpan materialSpan{modelsUbo, sizeof(vzt::Vec4), i * uniformByteNb + modelsAlignment};

        vzt::IndexedDescriptor ubos{};
        ubos[0] = vzt::DescriptorBuffer{vzt::DescriptorType::UniformBuffer, modelSpan};
        ubos[1] = vzt::DescriptorBuffer{vzt::DescriptorType::UniformBuffer, materialSpan};
        descriptorPool.update(i, ubos);

        createRenderObject(i);
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
    camera.front = vzt::Vec3(0.f, 0.f, 1.f);
    camera.up    = vzt::Vec3(0.f, 1.f, 0.f);
    camera.right = vzt::Vec3(1.f, 0.f, 0.f);

    const vzt::Vec3 target   = (minimum + maximum) * .5f;
    const float     bbRadius = glm::compMax(glm::abs(maximum - target));
    const float     distance = bbRadius / std::tan(camera.fov * .5f);
    const vzt::Vec3 position = target - camera.front * 1.15f * distance;

    // Actual rendering
    while (window.update())
    {
        const auto& inputs = window.getInputs();
        if (inputs.windowResized)
            swapchain.setExtent(inputs.windowSize);

        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        // Per frame update
        vzt::Quat orientation = {1.f, 0.f, 0.f, 0.f};

        float t = std::fmod(static_cast<float>(inputs.time) * 1e-3f, vzt::Tau);
        if (inputs.mouseLeftPressed)
            t = inputs.mousePosition.x * vzt::Tau / static_cast<float>(window.getWith());

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
        modelsUbo.update<vzt::Mat4>(matrices, submission->imageId * uniformByteNb);

        // Submission of pre-recorded commands
        vzt::CommandBuffer commands = commandPool[submission->imageId];
        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
        {
            // Wait all commands execution
            device.wait();

            // Apply screen size update
            extent             = window.getExtent();
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
