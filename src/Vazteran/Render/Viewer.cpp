//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#include "Vazteran/Render/Viewer.hpp"

namespace vzt
{
    Viewer::Viewer()
    {
        m_renderer = std::make_unique<Renderer>(m_window.Extensions());
    }

    void Viewer::Run()
    {
        while(!m_window.PollEvent());
    }

    Viewer::~Viewer()
    {

    }
}