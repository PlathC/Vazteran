#include <stdexcept>

#include "Vazteran/Window.hpp"

namespace vzt {
    Window::Window(Instance* instance, std::string_view name, const uint32_t width, const uint32_t height):
            m_instance(instance), m_width(width), m_height(height) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_window = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>(
                glfwCreateWindow(m_width, m_height, std::string(name).c_str(), nullptr, nullptr),
                [](GLFWwindow* window) {
                    glfwDestroyWindow(window);
                }
        );

        if (glfwCreateWindowSurface(m_instance->Get(), m_window.get(), nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        m_device = std::make_unique<vzt::DeviceManager>(m_instance, surface);
    }
}

