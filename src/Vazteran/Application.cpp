#include <memory>
#include <stdexcept>

#include <glm/gtc/random.hpp>

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

        std::vector<vzt::AABB> boudingBoxes;
        auto size = m_window->FrameBufferSize();
        m_models.emplace_back(std::make_unique<vzt::Model>("./samples/TheCrounchingBoy.obj"));
        boudingBoxes.emplace_back(m_models[0]->BoundingBox());
        for (uint32_t i = 0; i < 128; i++) {
            auto moved = std::make_unique<vzt::Model>(*m_models[0]);
            moved->Position() += glm::sphericalRand(5.);
            moved->Mesh().Materials()[0].ambientColor = glm::vec4(glm::sphericalRand(1.), 1.);
            boudingBoxes.emplace_back(moved->BoundingBox());

            m_models.emplace_back(std::move(moved));
        }

        vzt::Camera camera = Camera::FromModel(vzt::AABB(boudingBoxes), size.width / static_cast<float>(size.height));

        std::vector<vzt::Model*> renderedModel;
        renderedModel.reserve(m_models.size());
        for (const auto& model: m_models) {
            renderedModel.emplace_back(model.get());
        }

        m_renderer = std::make_unique<vzt::Renderer>(
                m_instance.get(), m_window->Surface(m_instance.get()), m_window->FrameBufferSize(), camera,
                std::move(renderedModel));
    }

    void Application::Run() {
        while(!m_window->Update()) {
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            for (auto& model: m_models) {
                model->Rotation() = time * glm::radians(45.0f) * glm::vec3(0.0f, 0.0f, 1.0f);
            }

            m_renderer->Draw();
        }

        vkDeviceWaitIdle(m_renderer->Device()->VkHandle());
    }

    Application::~Application() {
        glfwTerminate();
    }
}