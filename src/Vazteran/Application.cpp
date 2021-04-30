#include <iostream>
#include <memory>
#include <stdexcept>

#include "Vazteran/Application.hpp"
#include "Vazteran/Window.hpp"
#include "Vazteran/Vulkan/DeviceManager.hpp"

namespace vzt {
    Application::Application(std::string_view name) {
        if(!glfwInit())
            throw std::runtime_error("Failed to load glfw.");

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions;
        extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

        m_instance = std::make_unique<vzt::Instance>(name, extensions);
        m_window = std::make_unique<vzt::Window>(m_instance.get(), name, 800, 600);
    }

    void Application::Run() {
        while(!m_window->ShouldClose()) {
            glfwPollEvents();
            m_window->Draw();
        }

        vkDeviceWaitIdle(m_window->Device()->LogicalDevice());
    }

    Application::~Application() {
        glfwTerminate();
    }
}