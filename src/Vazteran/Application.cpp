#include <memory>
#include <random>
#include <stdexcept>

#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Application.hpp"
#include "Vazteran/Renderer.hpp"
#include "Vazteran/Window.hpp"

namespace vzt {
    Application::Application(std::string_view name, vzt::Scene scene) :
            m_scene(std::move(scene)) {
        m_window = std::make_unique<vzt::Window>(name, 800, 600, [&]() {
            if(m_renderer) {
                m_renderer->FrameBufferResized(m_window->FrameBufferSize());
                const auto size = m_window->FrameBufferSize();

                m_scene.SceneCamera().aspectRatio = size.width / static_cast<float>(size.height);
            }
        });

        m_instance = std::make_unique<vzt::Instance>(name, m_window->VkExtensions());

        const auto size = m_window->FrameBufferSize();
        m_scene.SceneCamera().aspectRatio = size.width / static_cast<float>(size.height);

        m_renderer = std::make_unique<vzt::Renderer>(m_instance.get(), m_window->Surface(m_instance.get()), 
            m_window->FrameBufferSize(), m_scene.SceneCamera(), m_scene.Models());
    }

    void Application::Run() {
        while(!m_window->Update()) {
            m_scene.Update();
            m_renderer->Draw();
        }

        vkDeviceWaitIdle(m_renderer->Device()->VkHandle());
    }

    Application::~Application() {
        glfwTerminate();
    }
}