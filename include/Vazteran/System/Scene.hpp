#ifndef VAZTERAN_SCENE_HPP
#define VAZTERAN_SCENE_HPP

#include "Vazteran/Backend/Vulkan/RenderGraph.hpp"
#include "Vazteran/Core/Event.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/System/System.hpp"

#include <functional>
#include <vector>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

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

		template <class EventType, auto Candidate, typename... Type>
		Connection onConstruct(Type&&... valueOrInstance);

		template <class EventType, auto Candidate, typename... Type>
		Connection onUpdate(Type&&... valueOrInstance);

		template <class EventType, auto Candidate, typename... Type>
		Connection onDestroy(Type&&... valueOrInstance);

		static Scene defaultScene(DefaultScene defaultScene);

	  private:
		mutable entt::registry m_registry{};
	};
} // namespace vzt

#include "Vazteran/System/Scene.inl"

#endif // VAZTERAN_SCENE_HPP
