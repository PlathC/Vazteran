//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#ifndef VAZTERAN_WINDOW_HPP
#define VAZTERAN_WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <memory>

namespace vzt
{
    class Window
    {
    public:
        Window(std::string_view windowTitle, uint32_t width, uint32_t height);

        std::vector<const char*> Extensions();

        bool PollEvent();

        ~Window();
    private:
        std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> m_handler;

        const uint32_t m_width;
        const uint32_t m_height;
    };

}

#endif //VAZTERAN_WINDOW_HPP
