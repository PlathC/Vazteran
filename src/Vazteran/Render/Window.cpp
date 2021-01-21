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
                [](GLFWwindow* handler) -> void
                {
                    glfwDestroyWindow(handler);
                }
        );

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        m_application = std::make_unique<Application>(
                std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount),
                [this](VkInstance instance, VkSurfaceKHR& surface)
                {
                    if (glfwCreateWindowSurface(instance, m_handler.get(), nullptr, &surface) != VK_SUCCESS)
                    {
                        throw std::runtime_error("failed to create window surface!");
                    }
                }
        );

        std::unique_ptr<VkSurfaceKHR> surface;


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