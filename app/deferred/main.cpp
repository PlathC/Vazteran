#include <vzt/Data/Camera.hpp>
#include <vzt/Data/Mesh.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/MeshLoader.hpp>
#include <vzt/Utils/RenderGraph.hpp>
#include <vzt/Vulkan/Pipeline/ComputePipeline.hpp>
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

    vzt::BufferBarrier barrier{buffer, vzt::Access::TransferWrite, vzt::Access::UniformRead};
    commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader, barrier);
}

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Deferred + Instancing + Compute";

    constexpr uint32_t InstanceCount = 32;

    auto compiler = vzt::Compiler();

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder::standard(), surface);
    auto queue     = device.getQueue(vzt::QueueType::Graphics);
    auto hardware  = device.getHardware();
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    vzt::Mesh mesh = vzt::readObj("samples/Dragon/dragon.obj");

    auto graph = vzt::RenderGraph{swapchain};

    auto instanceData = graph.addStorage(
        vzt::StorageBuilder{device, sizeof(vzt::Vec4f) * InstanceCount, vzt::BufferUsage::StorageBuffer});

    auto& instanceGeneration       = graph.addPass("InstanceGeneration", queue, vzt::PassType::Compute);
    auto& instanceGenerationLayout = instanceGeneration.getDescriptorLayout();
    instanceGenerationLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
    instanceGeneration.addStorageOutput(1, instanceData);

    auto computeProgram = vzt::Program(device);
    computeProgram.setShader(compiler.compile("shaders/deferred/instance_generation.comp", vzt::ShaderStage::Compute));

    auto computePipeline = vzt::ComputePipeline(device);
    computePipeline.setProgram(computeProgram);
    computePipeline.setDescriptorLayout(instanceGenerationLayout);
    computePipeline.compile();

    instanceGeneration.setRecordFunction<vzt::LambdaRecorder>(
        [&](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
            commands.bind(computePipeline, set);
            commands.dispatch(static_cast<uint32_t>(std::ceil(InstanceCount / static_cast<float>(InstanceCount))));
        });

    auto position = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment, vzt::Format::R32G32B32A32SFloat});
    auto normal   = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment, vzt::Format::R8G8B8A8SNorm});
    auto depth    = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry       = graph.addPass("Geometry", queue);
    auto& geometryLayout = geometry.getDescriptorLayout();
    geometryLayout.addBinding(0, vzt::DescriptorType::UniformBuffer);
    geometry.addStorageInput(1, instanceData);
    geometry.addColorOutput(position);
    geometry.addColorOutput(normal);
    geometry.setDepthOutput(depth);

    auto geometryProgram = vzt::Program(device);
    geometryProgram.setShader(compiler.compile("shaders/deferred/triangle.vert", vzt::ShaderStage::Vertex));
    geometryProgram.setShader(compiler.compile("shaders/deferred/triangle.frag", vzt::ShaderStage::Fragment));

    auto geometryPipeline = vzt::GraphicPipeline(device);
    geometryPipeline.setViewport(vzt::Viewport{window.getExtent()});
    geometryPipeline.setProgram(geometryProgram);
    geometryPipeline.setDescriptorLayout(geometryLayout);

    vzt::VertexInputDescription vertexDescription{};
    struct VertexInput
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
    vertexDescription.add(vzt::VertexAttribute{0, 0, vzt::Format::R32G32B32SFloat, 0}); // Position
    vertexDescription.add(
        vzt::VertexAttribute{offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0}); // Normal
    geometryPipeline.setVertexInputDescription(vertexDescription);

    auto& geometryRazterization    = geometryPipeline.getRasterization();
    geometryRazterization.cullMode = vzt::CullMode::None;

    geometry.setRecordFunction<vzt::LambdaRecorder>(
        [&](uint32_t i, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
            commands.bind(geometryPipeline, set);
            commands.bindVertexBuffer(vertexBuffer);
            for (const auto& subMesh : mesh.subMeshes)
                commands.drawIndexed(indexBuffer, subMesh.indices, InstanceCount);
        });

    auto  composed      = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment});
    auto  composedDepth = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});
    auto& shading       = graph.addPass("Shading", queue);

    auto& shadingLayout = shading.getDescriptorLayout();
    shading.addColorInput(0, position);
    shading.addColorInput(1, normal);
    shading.addColorOutput(composed);
    shading.setDepthOutput(composedDepth);

    auto shadingProgram = vzt::Program(device);
    shadingProgram.setShader(compiler.compile("shaders/deferred/full_screen.vert", vzt::ShaderStage::Vertex));
    shadingProgram.setShader(
        compiler.compile("shaders/deferred/deferred_blinn_phong.frag", vzt::ShaderStage::Fragment));

    auto shadingPipeline = vzt::GraphicPipeline(device);
    shadingPipeline.setProgram(shadingProgram);
    shadingPipeline.setViewport(vzt::Viewport{window.getExtent()});
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
    const std::size_t modelsAlignment = hardware.getUniformAlignment(sizeof(vzt::Mat4) * 3);
    vzt::Buffer       modelsUbo{device, modelsAlignment * swapchain.getImageNb(), vzt::BufferUsage::UniformBuffer,
                          vzt::MemoryLocation::Device, true};

    const std::size_t generationAlignment = hardware.getUniformAlignment(sizeof(vzt::Vec2u));
    vzt::Buffer generationUbo{device, generationAlignment * swapchain.getImageNb(), vzt::BufferUsage::UniformBuffer,
                              vzt::MemoryLocation::Device, true};

    // Assign buffer parts to their respective image
    vzt::DescriptorPool& instanceGenerationDescriptorPool = instanceGeneration.getDescriptorPool();
    vzt::DescriptorPool& geometryDescriptorPool           = geometry.getDescriptorPool();

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue, swapchain.getImageNb());
    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
    {
        vzt::BufferSpan        generationSpan{generationUbo, sizeof(vzt::Vec2u), i * generationAlignment};
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

        vzt::CommandBuffer commands = commandPool[submission->imageId];
        commands.begin();

        const vzt::Vec2u generationInput{InstanceCount, inputs.time};
        std::size_t      offset = submission->imageId * generationAlignment;
        updateUniformBuffer(commands, generationUbo, offset, &generationInput);

        offset = submission->imageId * modelsAlignment;
        updateUniformBuffer(commands, modelsUbo, offset, matrices.data(), matrices.size());

        graph.record(submission->imageId, commands);
        commands.end();

        graphicsQueue->submit(commands, *submission);
        if (!swapchain.present())
        {
            // Wait all commands execution
            device.wait();

            // Apply screen size update
            vzt::Extent2D extent = window.getExtent();
            camera.aspectRatio   = static_cast<float>(extent.width) / static_cast<float>(extent.height);

            geometryPipeline.resize(vzt::Viewport{extent});
            shadingPipeline.resize(vzt::Viewport{extent});
            graph.resize(extent);
        }
    }
}
