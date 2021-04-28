#include <cstdlib>
#include <iostream>

#include "Vazteran/Application.hpp"

int main()
{
    auto app = vzt::Application{"Vazteran"};

    try {
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

