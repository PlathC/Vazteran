//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#ifndef VAZTERAN_VIEWER_HPP
#define VAZTERAN_VIEWER_HPP

#include "Render/Window.hpp"

namespace vzt
{
    class Viewer
    {
    public:
        Viewer();

        void Run();

        ~Viewer();
    private:
        Window m_window{"Vazteran", 720, 480};
    };
}

#endif //VAZTERAN_VIEWER_HPP
