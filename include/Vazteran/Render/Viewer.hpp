//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#ifndef VAZTERAN_VIEWER_HPP
#define VAZTERAN_VIEWER_HPP

#include "Vazteran/Render/Window.hpp"
#include "Vazteran/Render/Renderer.hpp"

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
        std::unique_ptr<Renderer> m_renderer;
    };
}

#endif //VAZTERAN_VIEWER_HPP
