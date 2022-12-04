#include <vzt/Utils/RenderGraph.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

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
    auto graph        = vzt::RenderGraph{swapchain};

    auto albedo = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment, vzt::Format::B8G8R8A8SRGB});
    auto depth  = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});

    auto& geometry = graph.addPass("Geometry", graphicQueue);
    geometry.addColorOutput(albedo);
    geometry.setDepthOutput(depth);
    geometry.getDescriptorLayout().addBinding(0, vzt::DescriptorType::UniformBuffer);

    auto  composed      = graph.addAttachment({device, vzt::ImageUsage::ColorAttachment});
    auto  composedDepth = graph.addAttachment({device, vzt::ImageUsage::DepthStencilAttachment});
    auto& shading       = graph.addPass("Shading", graphicQueue);

    shading.getDescriptorLayout().addBinding(0, vzt::DescriptorType::UniformBuffer);
    shading.addColorInput(1, albedo);
    shading.addColorInput(2, depth);
    shading.addColorOutput(composed);
    shading.setDepthOutput(composedDepth);

    graph.setBackBuffer(composed);
    graph.compile();
}
