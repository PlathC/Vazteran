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

        m_models.emplace_back(std::make_unique<vzt::Model>("./samples/TheCrounchingBoy.obj"));
        // m_models[0]->Mat().diffuseMap = vzt::Image("./samples/viking_room.png");
        // m_models[0]->Mat().ambientMap = vzt::Image("./samples/viking_room.png");
        // m_models[0]->Mat().specularMap = vzt::Image("./samples/viking_room.png");

        auto size = m_window->FrameBufferSize();
        vzt::Camera camera = Camera::FromModel(*m_models[0], size.width / static_cast<float>(size.height));

        m_renderer = std::make_unique<vzt::Renderer>(
                m_instance.get(), m_window->Surface(m_instance.get()), m_window->FrameBufferSize(), camera,
                std::vector<vzt::Model*>{ m_models[0].get() });
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