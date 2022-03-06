#ifndef VAZTERAN_SCENE_HPP
#define VAZTERAN_SCENE_HPP

#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"

#include <functional>
#include <optional>
#include <vector>

#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Data/Model.hpp"
#include "Vazteran/Ui/UiManager.hpp"

namespace vzt
{
	class Scene
	{
	  public:
		enum class DefaultScene
		{
			CrounchingBoys,
			VikingRoom,
			MoriKnob
		};

	  public:
		Scene(std::vector<std::unique_ptr<Model>> models, Camera camera);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Scene(Scene&&) = default;
		Scene& operator=(Scene&&) = default;

		std::vector<vzt::Model*>           cModels() const;
		std::optional<vzt::ui::UiManager>& sceneUi() { return m_uiManager; }
		std::optional<vzt::ui::UiManager>  cSceneUi() const { return m_uiManager; }
		vzt::Camera&                       sceneCamera() { return m_camera; };
		vzt::Camera                        cSceneCamera() const { return m_camera; };

		void update() const;

		static Scene defaultScene(DefaultScene defaultScene);

	  private:
		vzt::Camera                              m_camera;
		std::vector<std::unique_ptr<vzt::Model>> m_models;
		std::optional<vzt::ui::UiManager>        m_uiManager;
	};
} // namespace vzt

#endif // VAZTERAN_SCENE_HPP
