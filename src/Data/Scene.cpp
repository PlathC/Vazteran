#include <iostream>

#include "Vazteran/Backend/Vulkan/UiRenderer.hpp"
#include "Vazteran/Core/Math.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Data/Scene.hpp"
#include "Vazteran/Views/MeshView.hpp"

namespace vzt
{
	Scene::Scene(std::vector<std::unique_ptr<Model>> models, Camera camera)
	    : m_models(std::move(models)), m_camera(std::move(camera))
	{
	}

	Scene ::~Scene() = default;

	std::vector<vzt::Model*> Scene::cModels() const
	{
		auto result = std::vector<vzt::Model*>();
		result.reserve(m_models.size());

		for (const auto& model : m_models)
			result.emplace_back(model.get());

		return result;
	}

	void Scene::update() const
	{
		for (const auto& model : m_models)
		{
			model->update();
		}
	}

	Scene Scene::defaultScene(const Scene::DefaultScene defaultScene)
	{
		switch (defaultScene)
		{
		case DefaultScene::CrounchingBoys: {
			constexpr std::size_t ModelNb = 64;

			std::vector<std::unique_ptr<vzt::Model>> models;
			models.reserve(ModelNb);

			models.emplace_back(std::make_unique<vzt::Model>("./samples/TheCrounchingBoy/TheCrounchingBoy.obj"));
			vzt::AABB fullBoundingBox = models[0]->boundingBox();

			vzt::ModelUpdateCallback modelUpdate = [](Model* model) {
				static auto startTime = std::chrono::high_resolution_clock::now();

				auto  currentTime = std::chrono::high_resolution_clock::now();
				float deltaTime =
				    std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
				model->rotation() = deltaTime * glm::radians(45.f) * glm::vec3(0.0f, 0.0f, 1.0f);

				const float timeValue =
				    std::chrono::duration<float, std::chrono::seconds::period>(currentTime.time_since_epoch()).count();
			};
			models[0]->setUpdateCallback(modelUpdate);

			for (std::size_t i = 1; i < ModelNb; i++)
			{
				auto moved    = std::make_unique<vzt::Model>(*models[0]);
				auto movement = glm::sphericalRand(5.f);

				moved->position() += movement;
				moved->mesh().materials()[1].color     = glm::vec4(((movement / 5.f) + 1.f) / 2.f, 1.f);
				moved->mesh().materials()[1].shininess = ((movement.z + 5.f) * 0.1f) * 200.f;
				moved->setUpdateCallback(modelUpdate);

				fullBoundingBox.extend(moved->boundingBox());
				models.emplace_back(std::move(moved));
			}

			vzt::ui::MainMenuField fileMenuField = vzt::ui::MainMenuField("File");
			fileMenuField.addItem(vzt::ui::MainMenuItem("Open", []() { std::cout << "Open" << std::endl; }));

			vzt::ui::MainMenuField brdfMenuField = vzt::ui::MainMenuField("BRDF");
			brdfMenuField.addItem(
			    vzt::ui::MainMenuItem("Blinn-Phong", []() { std::cout << "Blinn-Phong" << std::endl; }));

			vzt::ui::MainMenuBar mainMenuBar;
			mainMenuBar.addMenu(fileMenuField);
			mainMenuBar.addMenu(brdfMenuField);

			vzt::ui::UiManager uiManager;
			uiManager.setMainMenuBar(mainMenuBar);

			auto crounchingBoysScene        = Scene(std::move(models), Camera(fullBoundingBox));
			crounchingBoysScene.m_uiManager = std::move(uiManager);

			return crounchingBoysScene;
		}
		case DefaultScene::VikingRoom: {
			auto      vikingRoomModel = std::make_unique<vzt::Model>("./samples/VikingRoom/viking_room.obj");
			vzt::AABB fullBoundingBox = vikingRoomModel->boundingBox();

			auto tex = vzt::Image("./samples/viking_room.png");

			auto& mat   = vikingRoomModel->mesh().materials()[0];
			mat.texture = tex;

			vzt::ui::MainMenuField fileMenuField = vzt::ui::MainMenuField("File");
			fileMenuField.addItem(vzt::ui::MainMenuItem("Open", []() { std::cout << "Open" << std::endl; }));

			vzt::ui::MainMenuField brdfMenuField = vzt::ui::MainMenuField("BRDF");
			brdfMenuField.addItem(
			    vzt::ui::MainMenuItem("Blinn-Phong", []() { std::cout << "Blinn-Phong" << std::endl; }));

			vzt::ui::MainMenuBar mainMenuBar;
			mainMenuBar.addMenu(fileMenuField);
			mainMenuBar.addMenu(brdfMenuField);

			vzt::ui::UiManager uiManager;
			uiManager.setMainMenuBar(mainMenuBar);

			std::vector<std::unique_ptr<vzt::Model>> models;
			models.emplace_back(std::move(vikingRoomModel));

			auto vikingRoomScene        = Scene(std::move(models), Camera(fullBoundingBox));
			vikingRoomScene.m_uiManager = std::move(uiManager);

			return vikingRoomScene;
		}
		case DefaultScene::MoriKnob: {
			auto      moriKnobModel   = std::make_unique<vzt::Model>("./samples/MoriKnob/MoriKnob.obj");
			vzt::AABB fullBoundingBox = moriKnobModel->boundingBox();

			vzt::ui::MainMenuField fileMenuField = vzt::ui::MainMenuField("File");
			fileMenuField.addItem(vzt::ui::MainMenuItem("Open", []() { std::cout << "Open" << std::endl; }));

			vzt::ui::MainMenuField brdfMenuField = vzt::ui::MainMenuField("BRDF");
			brdfMenuField.addItem(
			    vzt::ui::MainMenuItem("Blinn-Phong", []() { std::cout << "Blinn-Phong" << std::endl; }));

			vzt::ui::MainMenuBar mainMenuBar;
			mainMenuBar.addMenu(std::move(fileMenuField));
			mainMenuBar.addMenu(std::move(brdfMenuField));

			vzt::ui::UiManager uiManager;
			uiManager.setMainMenuBar(mainMenuBar);

			std::vector<std::unique_ptr<vzt::Model>> models;
			models.emplace_back(std::move(moriKnobModel));

			Camera camera{};
			camera.position = vzt::Vec3(-0.011357f, 1.731663f, -4.142815f);
			camera.front    = vzt::Vec3(0.011475, -0.438221, 0.898794);

			auto moriKnowScene        = Scene(std::move(models), camera);
			moriKnowScene.m_uiManager = std::move(uiManager);

			return moriKnowScene;
		}
		default: throw std::runtime_error("The specified scene is not currently implemented.");
		}
	}
} // namespace vzt
