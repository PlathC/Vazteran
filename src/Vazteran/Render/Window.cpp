//
// Created by Cyprien Plateau--Holleville on 20/01/2021.
//

#include "Vazteran/Render/Window.hpp"

namespace vzt
{
    Window::Window(const std::string_view windowTitle, const uint32_t width, const uint32_t height) :
        m_width(width),
        m_height(height)
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_handler = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>(
                glfwCreateWindow(m_width, m_height, windowTitle.data(), nullptr, nullptr),
                [](GLFWwindow* handler) -> void {
                    glfwDestroyWindow(handler);
                }
        );

    }

    std::vector<const char*> Window::Extensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        auto result = std::vector<const char*>(glfwExtensionCount);

        for(uint32_t i = 0; i < glfwExtensionCount; i++)
        {
            result[i] = glfwExtensions[i];
        }

        return result;
    }

    bool Window::PollEvent()
    {
        glfwPollEvents();
        return glfwWindowShouldClose(m_handler.get());
    }

    Window::~Window()
    {
        glfwTerminate();
    }
}