#include <entt/entity/handle.hpp>

#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Data/Mesh.hpp"
#include "Vazteran/FileSystem/MeshReader.hpp"
#include "Vazteran/Math/Math.hpp"
#include "Vazteran/System/Scene.hpp"
#include "Vazteran/System/Transform.hpp"
#include "Vazteran/Views/MeshView.hpp"

namespace vzt
{
	Scene::Scene() {}

	Scene ::~Scene() = default;

	Entity Scene::create()
	{
		const entt::entity currentEntity = m_registry.create();
		return {m_registry, currentEntity};
	}

	Entity Scene::getMainCamera() const
	{
		auto cameras = m_registry.view<Camera, MainCamera>();
		for (const auto camera : cameras)
			return {m_registry, camera};
		throw std::runtime_error("Current scene does not hold any camera.");
	}

	Scene Scene::defaultScene(const Scene::DefaultScene defaultScene)
	{
		switch (defaultScene)
		{
		case DefaultScene::CrounchingBoys: {
			constexpr std::size_t ModelNb = 64;

			Scene crounchingBoysScene{};

			Mesh templateCrounchingBoy = vzt::readObj("./samples/TheCrounchingBoy/TheCrounchingBoy.obj");

			// vzt::ModelUpdateCallback modelUpdate = [](Model* model) {
			// 	static auto startTime = std::chrono::high_resolution_clock::now();
			//
			// 	auto  currentTime = std::chrono::high_resolution_clock::now();
			// 	float deltaTime =
			// 	    std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			// 	model->rotation() = deltaTime * glm::radians(45.f) * glm::vec3(0.0f, 0.0f, 1.0f);
			//
			// 	const float timeValue =
			// 	    std::chrono::duration<float, std::chrono::seconds::period>(currentTime.time_since_epoch()).count();
			// };

			AABB fullBoundingBox{};
			for (std::size_t i = 0; i < ModelNb; i++)
			{
				Entity     crounchingBoy = crounchingBoysScene.create();
				Mesh&      mesh          = crounchingBoy.emplace<Mesh>(templateCrounchingBoy);
				Transform& transform     = crounchingBoy.emplace<Transform>();

				auto movement = glm::sphericalRand(5.f);
				transform.position += movement;

				mesh.materials[1].color     = glm::vec4(((movement / 5.f) + 1.f) / 2.f, 1.f);
				mesh.materials[1].shininess = ((movement.z + 5.f) * 0.1f) * 200.f;

				const Mat4 transformationMatrix = transform.get();
				for (auto& position : mesh.aabb.getVertices())
					position = (transformationMatrix * Vec4(position, 1.f));
				mesh.aabb.refresh();

				fullBoundingBox.extend(mesh.aabb);
			}

			Entity cameraEntity = crounchingBoysScene.create();
			cameraEntity.emplace<Camera>(fullBoundingBox);
			cameraEntity.emplace<MainCamera>();

			return std::move(crounchingBoysScene);
		}
		case DefaultScene::VikingRoom: {

			Scene vikingRoomScene{};

			Entity room = vikingRoomScene.create();
			Mesh&  mesh = room.emplace<Mesh>(vzt::readObj("./samples/VikingRoom/viking_room.obj"));

			mesh.materials[0].texture = vzt::Image("./samples/VikingRoom/viking_room.png");

			Entity cameraEntity = vikingRoomScene.create();
			cameraEntity.emplace<Camera>(mesh.aabb);
			cameraEntity.emplace<MainCamera>();

			return std::move(vikingRoomScene);
		}
		case DefaultScene::MoriKnob: {
			Scene moriKnobScene{};

			Entity moriKnob = moriKnobScene.create();
			moriKnob.emplace<Mesh>(vzt::readObj("./samples/MoriKnob/MoriKnob.obj"));

			Entity  cameraEntity = moriKnobScene.create();
			Camera& camera       = cameraEntity.emplace<Camera>();
			camera.position      = vzt::Vec3(-0.011357f, 1.731663f, -4.142815f);
			camera.front         = vzt::Vec3(0.011475, -0.438221, 0.898794);
			cameraEntity.emplace<MainCamera>();

			return std::move(moriKnobScene);
		}
		default: throw std::runtime_error("The specified scene is not currently implemented.");
		}
	}
} // namespace vzt
