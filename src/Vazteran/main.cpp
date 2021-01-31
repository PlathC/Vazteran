//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "include/Vazteran/Viewer.hpp"

int main()
{
    try
    {
        vzt::Viewer app;
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

