#include <vzt/Data/Mesh.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/MeshLoader.hpp>
#include <vzt/Utils/RenderGraph.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

#include "vzt/Data/Camera.hpp"

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Deferred";

    auto window       = vzt::Window{ApplicationName};
    auto instance     = vzt::Instance{window};
    auto surface      = vzt::Surface{window, instance};
    auto device       = instance.getDevice(vzt::DeviceBuilder{}, surface);
    auto graphicQueue = device.getQueue(vzt::QueueType::Graphics);
    auto hardware     = device.getHardware();
    auto swapchain    = vzt::Swapchain{device, surface, window.getExtent()};

    vzt::Mesh mesh = vzt::readObj("samples/Dragon/dragon.obj");

    auto compiler = vzt::Compiler();

    auto graph = vzt::RenderGraph{swapchain};

    auto position = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment, vzt::Format::R32G32B32A32SFloat});
    auto normal   = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment, vzt::Format::R8G8B8A8SNorm});
    auto depth    = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry       = graph.addPass("Geometry", graphicQueue);
    auto& geometryLayout = geometry.getDescriptorLayout();
    geometryLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
    geometry.addColorOutput(position);
    geometry.addColorOutput(normal);
    geometry.setDepthOutput(depth);

    auto geometryProgram = vzt::Program(device);
    geometryProgram.setShader(compiler.compile("shaders/triangle.vert", vzt::ShaderStage::Vertex));
    geometryProgram.setShader(compiler.compile("shaders/triangle.frag", vzt::ShaderStage::Fragment));

    auto geometryPipeline = vzt::GraphicPipeline(device, geometryProgram, vzt::Viewport{window.getExtent()});
    geometryPipeline.setDescriptorLayout(geometryLayout);

    vzt::VertexInputDescription vertexDescription{};
    struct alignas(16) VertexInput
    {
        vzt::Vec3 inPosition;
        vzt::Vec3 inNormal;
    };
    std::vector<VertexInput> vertexInputs;
    vertexInputs.reserve(mesh.vertices.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
        vertexInputs.emplace_back(VertexInput{mesh.vertices[i], mesh.normals[i]});

    const auto vertexBuffer = vzt::Buffer::fromData<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer);
    const auto indexBuffer  = vzt::Buffer::fromData<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);

    vertexDescription.add(vzt::VertexBinding::Typed<VertexInput>(0));
    vertexDescription.add(vzt::VertexAttribute{0, 0, vzt::Format::R32G32B32SFloat, 0}); //
    vertexDescription.add(vzt::VertexAttribute{offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0});
    geometryPipeline.setVertexInputDescription(vertexDescription);

    auto& geometryRazterization    = geometryPipeline.getRasterization();
    geometryRazterization.cullMode = vzt::CullMode::None;

    geometry.setRecordFunction<vzt::LambdaRecorder>(
        [&geometryPipeline, &vertexBuffer, &indexBuffer, &mesh](uint32_t i, const vzt::DescriptorSet& set,
                                                                vzt::CommandBuffer& commands) {
            commands.bind(geometryPipeline, set);
            commands.bindVertexBuffer(vertexBuffer);
            for (const auto& subMesh : mesh.subMeshes)
                commands.drawIndexed(indexBuffer, subMesh.indices);
        });

    auto  composed      = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment});
    auto  composedDepth = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});
    auto& shading       = graph.addPass("Shading", graphicQueue);

    auto& shadingLayout = shading.getDescriptorLayout();
    shading.addColorInput(0, position);
    shading.addColorInput(1, normal);
    shading.addColorOutput(composed);
    shading.setDepthOutput(composedDepth);

    auto shadingProgram = vzt::Program(device);
    shadingProgram.setShader(compiler.compile("shaders/full_screen.vert", vzt::ShaderStage::Vertex));
    shadingProgram.setShader(compiler.compile("shaders/deferred_blinn_phong.frag", vzt::ShaderStage::Fragment));

    auto shadingPipeline = vzt::GraphicPipeline(device, shadingProgram, vzt::Viewport{window.getExtent()});
    shadingPipeline.setDescriptorLayout(shadingLayout);

    auto& shadingRazterization     = shadingPipeline.getRasterization();
    shadingRazterization.cullMode  = vzt::CullMode::Front;
    shadingRazterization.frontFace = vzt::FrontFace::CounterClockwise;

    shading.setRecordFunction<vzt::LambdaRecorder>(
        [&shadingPipeline](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
            commands.bind(shadingPipeline, set);
            commands.draw(3);
        });

    graph.setBackBuffer(composed);
    graph.compile();

    geometryPipeline.compile(geometry.getRenderPass());
    shadingPipeline.compile(shading.getRenderPass());

    // Initialize buffer with default values
    const std::size_t modelsAlignment    = hardware.getUniformAlignment(sizeof(vzt::Mat4) * 3);
    const std::size_t materialsAlignment = hardware.getUniformAlignment<vzt::Vec4>();
    const std::size_t uniformByteNb      = modelsAlignment + materialsAlignment;
    vzt::Buffer       modelsUbo{device, uniformByteNb * swapchain.getImageNb(), vzt::BufferUsage::UniformBuffer};

    // Assign buffer parts to their respective image
    vzt::DescriptorPool& geometryDescriptorPool = geometry.getDescriptorPool();

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        modelsUbo.update<vzt::Vec4>(vzt::Vec4{1.f}, i * uniformByteNb + modelsAlignment);

        vzt::BufferSpan modelSpan{modelsUbo, sizeof(vzt::Mat4) * 3u, i * uniformByteNb};

        vzt::IndexedDescriptor ubos{};
        ubos[0] = vzt::DescriptorBuffer{vzt::DescriptorType::UniformBuffer, modelSpan};
        geometryDescriptorPool.update(i, ubos);

        vzt::CommandBuffer commands = commandPool[i];
        commands.begin();
        graph.record(i, commands);
        commands.end();
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

    const vzt::Vec3 target         = (minimum + maximum) * .5f;
    const float     bbRadius       = glm::compMax(glm::abs(maximum - target));
    const float     distance       = bbRadius / std::tan(camera.fov * .5f);
    const vzt::Vec3 cameraPosition = target - camera.front * 1.15f * distance;

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
        const vzt::Vec3 currentPosition = rotation * (cameraPosition - target) + target;

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
            device.wait();

            // Apply screen size update
            vzt::Extent2D extent = window.getExtent();
            camera.aspectRatio   = static_cast<float>(extent.width) / static_cast<float>(extent.height);

            geometryPipeline = vzt::GraphicPipeline(device, geometryProgram, vzt::Viewport{window.getExtent()});
            geometryPipeline.setDescriptorLayout(geometryLayout);
            geometryPipeline.setVertexInputDescription(vertexDescription);
            geometryPipeline.compile(*geometry.getRenderPass());

            shadingPipeline = vzt::GraphicPipeline(device, shadingProgram, vzt::Viewport{window.getExtent()});
            shadingPipeline.setDescriptorLayout(shadingLayout);
            shadingPipeline.compile(*shading.getRenderPass());
        }
    }
}
