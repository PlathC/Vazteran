#include <vzt/Utils/MeshLoader.hpp>
#include <vzt/Vulkan/Surface.hpp>
#include <vzt/Vulkan/Swapchain.hpp>
#include <vzt/Window.hpp>

int main(int argc, char** argv)
{
    const std::string ApplicationName = "Vazteran Raytracing";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceBuilder{}, surface);
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};
}
