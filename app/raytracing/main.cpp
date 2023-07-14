#include <vzt/Data/Camera.hpp>
#include <vzt/Utils/Compiler.hpp>
#include <vzt/Utils/IOMesh.hpp>
#include <vzt/Vulkan/AccelerationStructure.hpp>
#include <vzt/Vulkan/Buffer.hpp>
#include <vzt/Vulkan/Command.hpp>
#include <vzt/Vulkan/Pipeline/RaytracingPipeline.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

struct VertexInput
{
    vzt::Vec3 inPosition;
    vzt::Vec3 inNormal;
};

int main(int argc, char** argv)
{
    const std::string ApplicationName = "Vazteran Raytracing";

    auto window   = vzt::Window{ApplicationName};
    auto instance = vzt::Instance{window};
    auto surface  = vzt::Surface{window, instance};
    auto compiler = vzt::Compiler(instance);

    auto device    = instance.getDevice(vzt::DeviceBuilder::rt(), surface);
    auto hardware  = device.getHardware();
    auto swapchain = vzt::Swapchain{device, surface};

    vzt::Mesh                mesh = vzt::readObj("samples/Dragon/dragon.obj");
    std::vector<VertexInput> vertexInputs;
    vertexInputs.reserve(mesh.vertices.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
        vertexInputs.emplace_back(VertexInput{mesh.vertices[i], mesh.normals[i]});

    constexpr vzt::BufferUsage GeometryBufferUsages =               //
        vzt::BufferUsage::AccelerationStructureBuildInputReadOnly | //
        vzt::BufferUsage::ShaderDeviceAddress |                     //
        vzt::BufferUsage::StorageBuffer;
    const auto vertexBuffer = vzt::Buffer::fromData<VertexInput>(   //
        device, vertexInputs, vzt::BufferUsage::VertexBuffer | GeometryBufferUsages);
    const auto indexBuffer  = vzt::Buffer::fromData<uint32_t>(      //
        device, mesh.indices, vzt::BufferUsage::IndexBuffer | GeometryBufferUsages);

    vzt::GeometryAsBuilder bottomAsBuilder{vzt::AsTriangles{
        vzt::Format::R32G32B32SFloat,
        vzt::BufferCSpan(vertexBuffer, vertexBuffer.size()),
        sizeof(VertexInput),
        vertexInputs.size(),
        vzt::BufferCSpan(indexBuffer, indexBuffer.size()),
    }};

    const auto bottomAs = vzt::AccelerationStructure( //
        device, bottomAsBuilder, vzt::AccelerationStructureType::BottomLevel);
    {
        auto scratchBuffer = vzt::Buffer{
            device,
            bottomAs.getScratchBufferSize(),
            vzt::BufferUsage::StorageBuffer | vzt::BufferUsage::ShaderDeviceAddress,
        };

        // "vkCmdBuildAccelerationStructuresKHR Supported Queue Types: Compute"
        const auto queue = device.getQueue(vzt::QueueType::Compute);
        queue->oneShot([&](vzt::CommandBuffer& commands) {
            vzt::AccelerationStructureBuilder builder{
                vzt::BuildAccelerationStructureFlag::PreferFastBuild,
                bottomAs,
                scratchBuffer,
            };
            commands.buildAs(builder);
        });
    }

    auto instancesData = std::vector{
        VkAccelerationStructureInstanceKHR{
            VkTransformMatrixKHR{
                1.f, 0.f, 0.f, 0.f, //
                0.f, 1.f, 0.f, 0.f, //
                0.f, 0.f, 1.f, 0.f, //
            },
            0,
            0xff,
            0,
            VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
            bottomAs.getDeviceAddress(),
        },
    };
    auto instances = vzt::Buffer::fromData<VkAccelerationStructureInstanceKHR>( //
        device, instancesData,
        vzt::BufferUsage::AccelerationStructureBuildInputReadOnly | vzt::BufferUsage::ShaderDeviceAddress);

    vzt::GeometryAsBuilder topAsBuilder{vzt::AsInstance{instances.getDeviceAddress(), 1}};
    const auto             topAs = vzt::AccelerationStructure( //
        device, topAsBuilder, vzt::AccelerationStructureType::TopLevel);
    {
        auto scratchBuffer = vzt::Buffer{
            device,
            topAs.getScratchBufferSize(),
            vzt::BufferUsage::StorageBuffer | vzt::BufferUsage::ShaderDeviceAddress,
        };

        // "vkCmdBuildAccelerationStructuresKHR Supported Queue Types: Compute"
        const auto queue = device.getQueue(vzt::QueueType::Compute);
        queue->oneShot([&](vzt::CommandBuffer& commands) {
            vzt::AccelerationStructureBuilder builder{
                vzt::BuildAccelerationStructureFlag::PreferFastBuild,
                topAs,
                scratchBuffer,
            };
            commands.buildAs(builder);
        });
    }

    vzt::ShaderGroup shaderGroup{device};
    shaderGroup.addShader(compiler.compile("shaders/raytracing/raygen.rgen", vzt::ShaderStage::RayGen));
    shaderGroup.addShader(compiler.compile("shaders/raytracing/miss.rmiss", vzt::ShaderStage::Miss));
    shaderGroup.addShader(compiler.compile("shaders/raytracing/closesthit.rchit", vzt::ShaderStage::ClosestHit),
                          vzt::ShaderGroupType::TrianglesHitGroup);

    vzt::DescriptorLayout layout{device};
    layout.addBinding(0, vzt::DescriptorType::AccelerationStructure);
    layout.addBinding(1, vzt::DescriptorType::StorageImage);
    layout.addBinding(2, vzt::DescriptorType::UniformBuffer);
    layout.compile();

    vzt::RaytracingPipeline pipeline{device};
    pipeline.setDescriptorLayout(layout);
    pipeline.setShaderGroup(shaderGroup);
    pipeline.compile();

    vzt::DescriptorPool descriptorPool{device, layout};
    descriptorPool.allocate(swapchain.getImageNb(), layout);

    const std::size_t uboAlignment = 2 * hardware.getUniformAlignment<vzt::Mat4>();
    vzt::Buffer       ubo{
        device, uboAlignment * swapchain.getImageNb(), vzt::BufferUsage::UniformBuffer, vzt::MemoryLocation::Device,
        true,
    };

    std::vector<vzt::DeviceImage> storageImages;
    std::vector<vzt::ImageView>   storageImageView;
    storageImages.reserve(swapchain.getImageNb());
    storageImageView.reserve(swapchain.getImageNb());

    auto       queue               = device.getQueue(vzt::QueueType::Graphics | vzt::QueueType::Compute);
    const auto createRenderObjects = [&](uint32_t i) {
        vzt::Extent2D extent = window.getExtent();

        storageImages.emplace_back( //
            device, extent, vzt::ImageUsage::Storage | vzt::ImageUsage::TransferSrc, vzt::Format::R32G32B32A32SFloat);

        const auto& lastImage = storageImages.back();
        queue->oneShot([&lastImage](vzt::CommandBuffer& commands) {
            vzt::ImageBarrier barrier{};
            barrier.image     = lastImage;
            barrier.oldLayout = vzt::ImageLayout::Undefined;
            barrier.newLayout = vzt::ImageLayout::General;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::BottomOfPipe, barrier);
        });

        storageImageView.emplace_back(vzt::ImageView{device, storageImages.back(), vzt::ImageAspect::Color});

        vzt::BufferSpan uboSpan{ubo, sizeof(vzt::Mat4) * 2u, i * uboAlignment};

        vzt::IndexedDescriptor ubos{};
        ubos[0] = vzt::DescriptorAccelerationStructure{vzt::DescriptorType::AccelerationStructure, topAs};
        ubos[1] = vzt::DescriptorImage{
            vzt::DescriptorType::StorageImage,
            storageImageView.back(),
            {},
            vzt::ImageLayout::General,
        };
        ubos[2] = vzt::DescriptorBuffer{vzt::DescriptorType::UniformBuffer, uboSpan};
        descriptorPool.update(i, ubos);
    };

    for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
        createRenderObjects(i);

    vzt::Buffer raygenShaderBindingTable{
        device,
        pipeline.getShaderHandleSize() + sizeof(vzt::Vec3),
        vzt::BufferUsage::ShaderBindingTable | vzt::BufferUsage::ShaderDeviceAddress,
        vzt::MemoryLocation::Device,
        true,
    };

    vzt::Buffer missShaderBindingTable{
        device,
        pipeline.getShaderHandleSize() + sizeof(vzt::Vec3),
        vzt::BufferUsage::ShaderBindingTable | vzt::BufferUsage::ShaderDeviceAddress,
        vzt::MemoryLocation::Device,
        true,
    };

    vzt::Buffer hitShaderBindingTable{
        device,
        pipeline.getShaderHandleSize() + sizeof(vzt::Vec3),
        vzt::BufferUsage::ShaderBindingTable | vzt::BufferUsage::ShaderDeviceAddress,
        vzt::MemoryLocation::Device,
        true,
    };

    const vzt::CSpan<uint8_t> shaderHandleStorage = pipeline.getShaderHandleStorage();
    const uint32_t            handleSizeAligned   = pipeline.getShaderHandleSizeAligned();
    const uint32_t            handleSize          = pipeline.getShaderHandleSize();

    vzt::Vec3 color1{0.5f, 0.5f, 0.5f};
    uint8_t*  rayGenData = raygenShaderBindingTable.map();
    std::memcpy(rayGenData, shaderHandleStorage.data, handleSize);
    std::memcpy(rayGenData + handleSize, &color1, sizeof(vzt::Vec3));
    raygenShaderBindingTable.unMap();

    vzt::Vec3 color2{1.f, 0.f, 0.f};
    uint8_t*  missData = missShaderBindingTable.map();
    std::memcpy(missData, shaderHandleStorage.data + handleSizeAligned, handleSize);
    std::memcpy(missData + handleSize, &color2, sizeof(vzt::Vec3));
    missShaderBindingTable.unMap();

    vzt::Vec3 color3{0.f, 1.f, 0.f};
    uint8_t*  hitData = hitShaderBindingTable.map();
    std::memcpy(hitData, shaderHandleStorage.data + 2u * handleSizeAligned, handleSize);
    std::memcpy(hitData + handleSize, &color3, sizeof(vzt::Vec3));
    hitShaderBindingTable.unMap();

    // Compute AABB to place camera in front of the model
    vzt::Vec3 minimum{std::numeric_limits<float>::max()};
    vzt::Vec3 maximum{std::numeric_limits<float>::lowest()};
    for (vzt::Vec3& vertex : mesh.vertices)
    {
        minimum = glm::min(minimum, vertex);
        maximum = glm::max(maximum, vertex);
    }

    vzt::Camera camera{};
    camera.up    = vzt::Vec3(0.f, 0.f, 1.f);
    camera.front = vzt::Vec3(0.f, 1.f, 0.f);
    camera.right = vzt::Vec3(1.f, 0.f, 0.f);

    const vzt::Vec3 target         = (minimum + maximum) * .5f;
    const float     bbRadius       = glm::compMax(glm::abs(maximum - target));
    const float     distance       = bbRadius / std::tan(camera.fov * .5f);
    const vzt::Vec3 cameraPosition = target - camera.front * 1.15f * distance;

    auto commandPool = vzt::CommandPool(device, queue, swapchain.getImageNb());

    float t = 0.f;
    while (window.update())
    {
        const auto& inputs = window.getInputs();
        if (inputs.windowResized)
            swapchain.recreate();

        auto submission = swapchain.getSubmission();
        if (!submission)
            continue;

        vzt::Extent2D extent = window.getExtent();

        // Per frame update
        vzt::Quat orientation = {1.f, 0.f, 0.f, 0.f};
        if (inputs.mouseLeftPressed)
            t = inputs.mousePosition.x * vzt::Tau / static_cast<float>(window.getWidth());

        const vzt::Quat rotation        = glm::angleAxis(t, camera.up);
        const vzt::Vec3 currentPosition = rotation * (cameraPosition - target) + target;

        vzt::Vec3       direction  = glm::normalize(target - currentPosition);
        const vzt::Vec3 reference  = camera.front;
        const float     projection = glm::dot(reference, direction);
        if (std::abs(projection) < 1.f - 1e-6f) // If direction and reference are not the same
            orientation = glm::rotation(reference, direction);
        else if (projection < 0.f)              // If direction and reference are opposite
            orientation = glm::angleAxis(-vzt::Pi, camera.up);

        vzt::Mat4                view = camera.getViewMatrix(currentPosition, orientation);
        std::array<vzt::Mat4, 2> matrices{glm::inverse(view), glm::inverse(camera.getProjectionMatrix())};

        const auto&        image    = swapchain.getImage(submission->imageId);
        vzt::CommandBuffer commands = commandPool[submission->imageId];
        {
            commands.begin();

            uint8_t* data = ubo.map();
            std::memcpy(data + submission->imageId * uboAlignment, matrices.data(), 2 * sizeof(vzt::Mat4));
            ubo.unMap();

            vzt::BufferBarrier barrier{ubo, vzt::Access::TransferWrite, vzt::Access::UniformRead};
            commands.barrier(vzt::PipelineStage::Transfer, vzt::PipelineStage::RaytracingShader, barrier);

            vzt::ImageBarrier imageBarrier{};
            imageBarrier.image     = storageImages[submission->imageId];
            imageBarrier.oldLayout = vzt::ImageLayout::Undefined;
            imageBarrier.newLayout = vzt::ImageLayout::General;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.bind(pipeline, descriptorPool[submission->imageId]);
            commands.traceRays({raygenShaderBindingTable.getDeviceAddress(), handleSizeAligned, handleSizeAligned},
                               {missShaderBindingTable.getDeviceAddress(), handleSizeAligned,
                                vzt::align(handleSizeAligned + sizeof(vzt::Vec3),
                                           std::max(handleSizeAligned, handleSizeAligned - handleSize))},
                               {hitShaderBindingTable.getDeviceAddress(), handleSizeAligned,
                                vzt::align(handleSizeAligned + sizeof(vzt::Vec3),
                                           std::max(handleSizeAligned, handleSizeAligned - handleSize))},
                               {}, extent.width, extent.height, 1);

            imageBarrier.image     = storageImages[submission->imageId];
            imageBarrier.oldLayout = vzt::ImageLayout::General;
            imageBarrier.newLayout = vzt::ImageLayout::TransferSrcOptimal;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::Undefined;
            imageBarrier.newLayout = vzt::ImageLayout::TransferDstOptimal;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.blit(storageImages[submission->imageId], vzt::ImageLayout::TransferSrcOptimal, image,
                          vzt::ImageLayout::TransferDstOptimal);

            imageBarrier.image     = image;
            imageBarrier.oldLayout = vzt::ImageLayout::TransferDstOptimal;
            imageBarrier.newLayout = vzt::ImageLayout::PresentSrcKHR;
            commands.barrier(vzt::PipelineStage::TopOfPipe, vzt::PipelineStage::Transfer, imageBarrier);

            commands.end();
        }

        queue->submit(commands, *submission);
        if (!swapchain.present())
        {
            // Wait all commands execution
            device.wait();

            // Apply screen size update
            extent             = window.getExtent();
            camera.aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

            storageImages.clear();
            storageImageView.clear();
            for (uint32_t i = 0; i < swapchain.getImageNb(); i++)
                createRenderObjects(i);
        }
    }

    return EXIT_SUCCESS;
}
