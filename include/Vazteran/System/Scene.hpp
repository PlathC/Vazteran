#ifndef VAZTERAN_SCENE_HPP
#define VAZTERAN_SCENE_HPP

#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"

#include <functional>
#include <optional>
#include <vector>

#include <entt/entity/entity.hpp>
#include <entt/entity/handle.hpp>
#include <entt/entity/registry.hpp>

#include "Vazteran/Data/Camera.hpp"

namespace vzt
{
	using Entity = entt::handle;

	template <class ComponentType>
	class Listener;

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
		Scene();

		Scene(const Scene&)            = default;
		Scene& operator=(const Scene&) = default;

		Scene(Scene&&)            = default;
		Scene& operator=(Scene&&) = default;

		~Scene();

		Entity create();
		Entity getMainCamera() const;

		using ForAllFunction        = std::function<void(Entity)>;
		using ForAllIndexedFunction = std::function<void(std::size_t Entity)>;

		template <class... ComponentTypes>
		void forAll(ForAllFunction forAllFunction);
		template <class... ComponentTypes>
		void forAll(ForAllIndexedFunction forallIndexedFunction);

		static Scene defaultScene(DefaultScene defaultScene);

		template <typename>
		friend class Listener;

	  private:
		mutable entt::registry m_registry{};
	};
} // namespace vzt

#include "Vazteran/System/Scene.inl"

#endif // VAZTERAN_SCENE_HPP
