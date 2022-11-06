#include <cstdlib>

#include <vzt/Window.hpp>

int main(int argc, char** argv)
{
    const std::string ApplicationName = "Vazteran Base";
    vzt::Window       window{ApplicationName};
    vzt::Instance     instance{window};
    vzt::Surface      surface{window, instance};

    return EXIT_SUCCESS;
}
