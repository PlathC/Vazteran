#include <cstdlib>

#include <vzt/Surface.hpp>
#include <vzt/Swapchain.hpp>

int main(int argc, char** argv)
{
    const std::string ApplicationName = "Vazteran Base";

    auto window    = vzt::Window{ApplicationName};
    auto instance  = vzt::Instance{window};
    auto surface   = vzt::Surface{window, instance};
    auto device    = instance.getDevice(vzt::DeviceConfiguration{}, surface);
    auto swapchain = vzt::Swapchain{device, surface, window.getExtent()};

    return EXIT_SUCCESS;
}
