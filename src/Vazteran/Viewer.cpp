//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#include "Vazteran/Viewer.hpp"

namespace vzt
{
    Viewer::Viewer()
    {
    }

    void Viewer::Run()
    {
        while(!m_window.PollEvent());
    }

    Viewer::~Viewer()
    {

    }
}