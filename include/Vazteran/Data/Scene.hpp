#ifndef VAZTERAN_SCENE_HPP
#define VAZTERAN_SCENE_HPP

#include <functional>
#include <vector>

#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Model.hpp"

namespace vzt
{
	using ModelUpdateCallback = std::function<void(Model*)>;

	class Scene
	{
	  public:
		enum class DefaultScene
		{
			CrounchingBoys
		};

	  public:
		Scene(std::vector<std::unique_ptr<Model>> models, Camera camera,
		      ModelUpdateCallback callback = ModelUpdateCallback());
		~Scene();

		Scene(Scene&) noexcept = delete;
		Scene& operator=(Scene&) noexcept = delete;

		Scene(Scene&&) noexcept = default;
		Scene& operator=(Scene&&) noexcept = default;

		Camera&             SceneCamera() { return m_camera; };
		Camera              CSceneCamera() const { return m_camera; };
		std::vector<Model*> Models() const;
		void                Update() const;

		static Scene Default(DefaultScene defaultScene);

	  private:
		std::vector<std::unique_ptr<Model>> m_models;
		ModelUpdateCallback                 m_callback;
		Camera                              m_camera;
	};
} // namespace vzt

#endif // VAZTERAN_SCENE_HPP
