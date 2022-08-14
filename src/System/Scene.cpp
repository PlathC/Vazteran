#include <entt/entity/handle.hpp>

#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Data/Light.hpp"
#include "Vazteran/Data/Mesh.hpp"
#include "Vazteran/Data/Transform.hpp"
#include "Vazteran/FileSystem/MeshReader.hpp"
#include "Vazteran/Math/Math.hpp"
#include "Vazteran/System/Scene.hpp"

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
		Scene scene{};

		Entity directionalLightEntity = scene.create();

		DirectionalLight light;
		light.direction = Vec4(1.f);
		light.color     = Vec4(1.f);
		directionalLightEntity.emplace<DirectionalLight>(light);
		switch (defaultScene)
		{
		case DefaultScene::CrounchingBoys: {
			constexpr std::size_t ModelNb = 64;

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
				Entity     crounchingBoy = scene.create();
				Mesh&      mesh          = crounchingBoy.emplace<Mesh>(templateCrounchingBoy);
				Transform& transform     = crounchingBoy.emplace<Transform>();

				auto movement = glm::sphericalRand(5.f);
				transform.position += movement;

				mesh.materials[1].color     = glm::vec4(((movement / 5.f) + 1.f) / 2.f, 1.f);
				mesh.materials[1].shininess = ((movement.z + 5.f) * 0.1f) * 200.f;

				const Mat4 transformationMatrix = transform.get();
				for (auto& position : mesh.aabb.vertices)
					position = (transformationMatrix * Vec4(position, 1.f));
				mesh.aabb.refresh();

				fullBoundingBox.extend(mesh.aabb);
			}

			Entity  cameraEntity = scene.create();
			Camera& camera       = cameraEntity.emplace<Camera>();
			cameraEntity.emplace<Transform>(vzt::fromAabb(camera, fullBoundingBox));
			cameraEntity.emplace<MainCamera>();
			break;
		}
		case DefaultScene::VikingRoom: {
			Entity room = scene.create();
			Mesh&  mesh = room.emplace<Mesh>(vzt::readObj("./samples/VikingRoom/viking_room.obj"));

			mesh.materials[0].texture = vzt::Image("./samples/VikingRoom/viking_room.png");

			Entity  cameraEntity = scene.create();
			Camera& camera       = cameraEntity.emplace<Camera>();
			cameraEntity.emplace<Transform>(fromAabb(camera, mesh.aabb));
			cameraEntity.emplace<MainCamera>();
			break;
		}
		case DefaultScene::MoriKnob: {
			Entity moriKnob = scene.create();
			moriKnob.emplace<Mesh>(vzt::readObj("./samples/MoriKnob/MoriKnob.obj"));

			Entity cameraEntity = scene.create();
			cameraEntity.emplace<Camera>();
			Transform& transform = cameraEntity.emplace<Transform>();
			transform.position   = Vec3(-0.21498573f, 1.1878444f, -1.884456f);
			transform.rotation   = glm::normalize(Quat(0.012553758f, -0.0049332553f, 0.46932048f, 0.88292503f));
			cameraEntity.emplace<MainCamera>();
			break;
		}
		default: throw std::runtime_error("The specified scene is not currently implemented.");
		}

		return scene;
	}
} // namespace vzt
