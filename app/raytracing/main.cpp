#include <vzt/Utils/MeshLoader.hpp>
#include <vzt/Vulkan/Buffer.hpp>
#include <vzt/Vulkan/Command.hpp>
#include <vzt/Vulkan/Raytracing/AccelerationStructure.hpp>
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

    auto device    = instance.getDevice(vzt::DeviceBuilder::rt(), surface);
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    vzt::Mesh                mesh = vzt::readObj("samples/Dragon/dragon.obj");
    std::vector<VertexInput> vertexInputs;
    vertexInputs.reserve(mesh.vertices.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
        vertexInputs.emplace_back(VertexInput{mesh.vertices[i], mesh.normals[i]});

    constexpr vzt::BufferUsage GeometryBufferUsages =               //
        vzt::BufferUsage::AccelerationStructureBuildInputReadOnly | //
        vzt::BufferUsage::ShaderDeviceAddress |                     //
        vzt::BufferUsage::StorageBuffer;
    const auto vertexBuffer = vzt::Buffer::fromData<VertexInput>( //
        device, vertexInputs, vzt::BufferUsage::VertexBuffer | GeometryBufferUsages);
    const auto indexBuffer  = vzt::Buffer::fromData<uint32_t>( //
        device, mesh.indices, vzt::BufferUsage::IndexBuffer | GeometryBufferUsages);

    vzt::GeometryAsBuilder bottomAsBuilder{vzt::AsTriangles{
        vzt::Format::R32G32B32SFloat,
        vertexBuffer,
        sizeof(vzt::Vec3),
        vertexInputs.size(),
        indexBuffer,
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
}
