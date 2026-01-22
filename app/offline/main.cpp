#include <fstream>
#include <queue>

#include <vzt/camera.hpp>
#include <vzt/compiler.hpp>
#include <vzt/core/logger.hpp>
#include <vzt/render_graph.hpp>
#include <vzt/vulkan/query_pool.hpp>
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
    const std::string ApplicationName = "Vazteran Offline";
    const uint32_t    Width           = 2048 * 4;
    const uint32_t    Height          = 1024 * 4;
    const vzt::Format Format          = vzt::Format::B8G8R8A8SRGB;

    auto instance      = vzt::Instance{ApplicationName};
    auto deviceBuilder = vzt::DeviceBuilder::offline();

    auto device   = instance.getDevice(deviceBuilder);
    auto compiler = vzt::Compiler(instance);
    auto graph    = vzt::RenderGraph{device};

    vzt::Mesh                mesh = vzt::readObj("samples/Dragon/dragon.obj");
    std::vector<VertexInput> vertexInputs;
    vertexInputs.reserve(mesh.vertices.size());

    vzt::Vec3 minimum{std::numeric_limits<float>::max()}, maximum{std::numeric_limits<float>::lowest()};
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
    {
        const vzt::Vec3 vertex = mesh.vertices[i];
        vertexInputs.emplace_back(VertexInput{vertex, mesh.normals[i]});
        minimum = glm::min(minimum, vertex);
        maximum = glm::max(maximum, vertex);
    }

    const auto vertexBuffer = vzt::Buffer::From<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer);
    const auto indexBuffer  = vzt::Buffer::From<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer);

    vzt::VertexInputDescription vertexDescription{};
    vertexDescription.add(vzt::VertexBinding::Typed<VertexInput>(0));
    vertexDescription.add(offsetof(VertexInput, inPosition), 0, vzt::Format::R32G32B32SFloat, 0); // Position
    vertexDescription.add(offsetof(VertexInput, inNormal), 1, vzt::Format::R32G32B32SFloat, 0);   // Normal

    // Draw geometry pass
    auto color = graph.addAttachment({vzt::ImageUsage::ColorAttachment});
    auto depth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry = graph.addGraphics("Geometry", compiler("shaders/offline/offline.slang"));
    {
        geometry.addColorOutput(color, "", vzt::Vec4(1.f, 0.91f, 0.69f, 1.f));
        geometry.setDepthOutput(depth);

        auto& geometryPipeline = geometry.getPipeline();
        geometryPipeline.setVertexInputDescription(vertexDescription);
        geometryPipeline.setViewport({Width, Height});

        auto& geometryRasterization    = geometryPipeline.getRasterization();
        geometryRasterization.cullMode = vzt::CullMode::None;

        geometry.setRecordFunction<vzt::LambdaRecorder>(
            [&](uint32_t, const vzt::DescriptorSet& set, vzt::CommandBuffer& commands) {
                commands.bind(geometryPipeline, set);
                commands.bindVertexBuffer(vertexBuffer);
                for (const auto& subMesh : mesh.subMeshes)
                    commands.drawIndexed(indexBuffer, subMesh.indices);
            });
    }

    vzt::DeviceImage output = {
        device,
        {
            .size   = {Width, Height},
            .usage  = vzt::ImageUsage::ColorAttachment | vzt::ImageUsage::TransferSrc,
            .format = Format,
        },
    };

    graph.setBackbuffer(output, vzt::ImageLayout::General, color);
    graph.compile();

    // Assign buffer parts to their respective image
    vzt::UniformBuffer ubo = {device, sizeof(vzt::Mat4) * 3, 1, true};

    auto graphicsQueue = device.getQueue(vzt::QueueType::Graphics);
    auto commandPool   = vzt::CommandPool(device, graphicsQueue);

    auto& descriptorPool = geometry.getDescriptorPool();
    descriptorPool.update(0, {{0, ubo.getDescriptor(0)}});

    // Compute AABB to place camera in front of the model
    vzt::Camera camera{};
    camera.up          = vzt::Vec3(0.f, 0.f, 1.f);
    camera.front       = vzt::Vec3(0.f, 1.f, 0.f);
    camera.right       = vzt::Vec3(1.f, 0.f, 0.f);
    camera.aspectRatio = static_cast<float>(Width) / static_cast<float>(Height);

    const vzt::Vec3 target   = (minimum + maximum) * .5f;
    const float     bbRadius = glm::compMax(glm::abs(maximum - target));

    std::vector<uint64_t> times{};
    times.resize((graph.size() + 1) * 2);

    // Actual rendering
    // Per frame update
    vzt::Quat orientation = {1.f, 0.f, 0.f, 0.f};

    float           t        = vzt::Tau * .75f;
    const vzt::Quat rotation = glm::angleAxis(t, camera.up);

    const float distanceScale = .45f;
    const float distance      = distanceScale * 2.f * bbRadius / std::tan(camera.fov * .5f);

    const vzt::Vec3 cameraPosition  = target - camera.front * distance;
    const vzt::Vec3 currentPosition = rotation * (cameraPosition - target) + target;

    const vzt::Vec3 direction  = glm::normalize(target - currentPosition);
    const vzt::Vec3 reference  = camera.front;
    const float     projection = glm::dot(reference, direction);
    if (std::abs(projection) < 1.f - 1e-6f) // If direction and reference are not the same
        orientation = glm::rotation(reference, direction);
    else if (projection < 0.f) // If direction and reference are opposite
        orientation = glm::angleAxis(-vzt::Pi, camera.up);

    const vzt::Mat4 view = camera.getViewMatrix(currentPosition, orientation);
    std::array      matrices{view, camera.getProjectionMatrix(), glm::transpose(glm::inverse(view))};

    vzt::CommandBuffer commands = commandPool[0];
    commands.begin();

    ubo.write(commands, matrices, 0);
    commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::VertexShader,
                     {ubo.getSpan(0), vzt::Access::TransferWrite, vzt::Access::UniformRead});

    for (uint32_t i = 0; i < graph.size(); i++)
    {
        auto& pass = graph[i];
        pass->record(0, commands);
    }

    commands.end();

    graphicsQueue->submit(commands);
    device.wait();

    vzt::DeviceImage targetImage = {
        device,
        vzt::ImageBuilder{
            .size     = {Width, Height},
            .usage    = vzt::ImageUsage::TransferDst,
            .format   = Format,
            .tiling   = vzt::ImageTiling::Linear,
            .mappable = true,
        },
    };

    {
        commands.begin();

        vzt::ImageBarrier transition{};
        transition.image     = output;
        transition.oldLayout = vzt::ImageLayout::General;
        transition.newLayout = vzt::ImageLayout::TransferSrcOptimal;
        commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::Transfer, transition);

        transition.image     = targetImage;
        transition.oldLayout = vzt::ImageLayout::Undefined;
        transition.newLayout = vzt::ImageLayout::TransferDstOptimal;
        commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::Transfer, transition);

        commands.copy(output, targetImage, Width, Height);

        transition.image     = targetImage;
        transition.oldLayout = vzt::ImageLayout::TransferDstOptimal;
        transition.newLayout = vzt::ImageLayout::General;
        commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::Transfer, transition);

        commands.end();
        graphicsQueue->submit(commands);
    };

    const vzt::SubresourceLayout subresourceLayout = targetImage.getSubresourceLayout(vzt::ImageAspect::Color);
    const uint8_t*               mappedData        = targetImage.map<uint8_t>();
    mappedData += subresourceLayout.offset;

    std::vector<char> out{};
    std::format_to(std::back_inserter(out), "P3\n{} {}\n255\n", Width, Height);

    std::vector<uint8_t> cpuData = std::vector<uint8_t>(Width * Height * 4);

    for (uint32_t y = 0; y < Height; y++)
    {
        const uint64_t dstStride = y * Width * sizeof(uint8_t) * 4;
        const uint64_t srcStride = y * subresourceLayout.rowPitch;
        std::memcpy(cpuData.data() + dstStride, mappedData + srcStride, Width * sizeof(uint8_t) * 4);

        for (uint32_t x = 0; x < Width; x++)
        {
            std::format_to(std::back_inserter(out), "{} {} {}\n", //
                           cpuData[(y * Width + x) * 4 + 2], cpuData[(y * Width + x) * 4 + 1],
                           cpuData[(y * Width + x) * 4 + 0]);
        }
    }
    targetImage.unmap();

    auto outputPpm      = std::ofstream{"output.ppm"};
    auto outputIterator = std::ostream_iterator<char>{outputPpm, ""};
    std::copy(out.begin(), out.end(), outputIterator);

    return EXIT_SUCCESS;
}
