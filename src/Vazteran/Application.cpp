#include <iostream>
#include <memory>
#include <stdexcept>

#include "Vazteran/Application.hpp"
#include "Vazteran/Renderer.hpp"
#include "Vazteran/Window.hpp"

namespace vzt {
    Application::Application(std::string_view name) {
        m_window = std::make_unique<vzt::Window>(name, 800, 600, [&]() {
            if(m_renderer) {
                m_renderer->FrameBufferResized(m_window->FrameBufferSize());
            }
        });
        m_instance = std::make_unique<vzt::Instance>(name, m_window->VkExtensions());

        std::unique_ptr<vzt::Model> model = std::make_unique<vzt::Model>("./samples/TheCrounchingBoy.obj");
        model->Mat().ambient = glm::vec4(0.5, 0.5, 0.5, 1.);
        model->Mat().diffuse = glm::vec4(0.5, 0.5, 0.5, 1.);
        model->Mat().specular = glm::vec4(0.5, 0.5, 0.5, 1.);

        auto size = m_window->FrameBufferSize();
        vzt::Camera camera = Camera::FromModel(*model, size.width / static_cast<float>(size.height));

        m_renderer = std::make_unique<vzt::Renderer>(
                m_instance.get(), m_window->Surface(m_instance.get()), m_window->FrameBufferSize(), std::move(model), camera);
    }

    void Application::Run() {
        while(!m_window->Update()) {
            m_renderer->Draw();
        }

        vkDeviceWaitIdle(m_renderer->Device()->VkHandle());
    }

    Application::~Application() {
        glfwTerminate();
    }
}