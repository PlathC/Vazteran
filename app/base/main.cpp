#include <cstdlib>

#include <vzt/Window.hpp>

int main(int argc, char** argv)
{
    const std::string ApplicationName = "Vazteran Base";

    auto window   = vzt::Window{ApplicationName};
    auto instance = vzt::Instance{window};
    auto surface  = vzt::Surface{window, instance};
    auto device   = instance.getDevice(vzt::DeviceConfiguration{}, surface);

    return EXIT_SUCCESS;
}
