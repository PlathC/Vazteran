#include <vzt/Utils/RenderGraph.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

int main(int /* argc */, char** /* argv */)
{
    const std::string ApplicationName = "Vazteran Deferred";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder{}, surface);
    auto hardware  = device.getHardware();
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    vzt::RenderGraph graph{};

    auto albedo = graph.addAttachment({vzt::ImageUsage::ColorAttachment, vzt::Format::B8G8R8A8SRGB});
    auto depth  = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});

    auto graphicQueue = device.getQueue(vzt::QueueType::Graphics);

    auto geometry = graph.addPass("Geometry", graphicQueue);
    geometry.addColorOutput(albedo);
    geometry.setDepthOutput(depth);

    auto composed      = graph.addAttachment({vzt::ImageUsage::ColorAttachment});
    auto composedDepth = graph.addAttachment({vzt::ImageUsage::DepthStencilAttachment});
    auto shading       = graph.addPass("Shading", graphicQueue);
    shading.addColorInput(albedo);
    shading.addColorInput(depth);
    shading.addColorOutput(composed);
    shading.setDepthOutput(composedDepth);

    graph.setBackBuffer(composed);
    graph.compile();
}
