#include <vzt/Utils/MeshLoader.hpp>
#include <vzt/Vulkan/Buffer.hpp>
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

    vzt::DeviceBuilder deviceBuilder{};
    deviceBuilder.extensions.emplace_back(vzt::dext::AccelerationStructure);
    deviceBuilder.extensions.emplace_back(vzt::dext::RaytracingPipeline);
    deviceBuilder.extensions.emplace_back(vzt::dext::BufferDeviceAddress);
    deviceBuilder.extensions.emplace_back(vzt::dext::DeferredHostOperations);
    deviceBuilder.extensions.emplace_back(vzt::dext::DescriptorIndexing);
    deviceBuilder.extensions.emplace_back(vzt::dext::Spirv14);
    deviceBuilder.extensions.emplace_back(vzt::dext::ShaderFloatControls);
    auto device    = instance.getDevice(deviceBuilder, surface);
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    vzt::Mesh                mesh = vzt::readObj("samples/Dragon/dragon.obj");
    std::vector<VertexInput> vertexInputs;
    vertexInputs.reserve(mesh.vertices.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++)
        vertexInputs.emplace_back(VertexInput{mesh.vertices[i], mesh.normals[i]});

    constexpr vzt::BufferUsage GeometryBufferUsages = vzt::BufferUsage::AccelerationStructureBuildInputReadOnly |
                                                      vzt::BufferUsage::ShaderDeviceAddress |
                                                      vzt::BufferUsage::StorageBuffer;
    const auto vertexBuffer =
        vzt::Buffer::fromData<VertexInput>(device, vertexInputs, vzt::BufferUsage::VertexBuffer | GeometryBufferUsages);
    const auto indexBuffer =
        vzt::Buffer::fromData<uint32_t>(device, mesh.indices, vzt::BufferUsage::IndexBuffer | GeometryBufferUsages);
}
