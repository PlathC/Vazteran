#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Scene.hpp"

namespace vzt {
	Scene::Scene(std::vector<std::unique_ptr<Model>> models, Camera camera, ModelUpdateCallback callback):
		m_models(std::move(models)), m_callback(callback), m_camera(std::move(camera)) {
	}

	std::vector<Model*> Scene::Models() const {
		auto result = std::vector<Model*>();
		result.reserve(m_models.size());
		
		for (const auto& model : m_models)
			result.emplace_back(model.get());

		return result;
	}

	void Scene::Update() const {
		for (const auto& model : m_models) {
			m_callback(model.get());
		}
	}

	Scene Scene::Default(Scene::DefaultScene defaultScene) {
		switch (defaultScene) {
			case DefaultScene::CrounchingBoys: {
				std::vector<std::unique_ptr<vzt::Model>> models;
				models.reserve(128);
				models.emplace_back(std::make_unique<vzt::Model>("./samples/TheCrounchingBoy.obj"));
				vzt::AABB fullBoundingBox = models[0]->BoundingBox();
				for (std::size_t i = 1; i < 128; i++) {
					auto moved = std::make_unique<vzt::Model>(*models[0]);
					auto movement = glm::sphericalRand(5.f);
					moved->Position() += movement;
					moved->Mesh().Materials()[0].ambientColor = glm::vec4(((movement / 5.f) + 1.f) / 2.f, 1.);
					fullBoundingBox.Extend(moved->BoundingBox());

					models.emplace_back(std::move(moved));
				}

				return Scene(std::move(models), Camera::FromBoundingBox(fullBoundingBox), [](Model* model) {
					static auto startTime = std::chrono::high_resolution_clock::now();

					auto currentTime = std::chrono::high_resolution_clock::now();
					float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
					model->Rotation() = time * glm::radians(45.f) * glm::vec3(0.0f, 0.0f, 1.0f);
				});
			}
			default: {
				throw std::runtime_error("The specified scene is not currently implemented.");
			}
		}
	}
}