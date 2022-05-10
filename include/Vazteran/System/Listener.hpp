#ifndef VAZTERAN_LISTENER_HPP
#define VAZTERAN_LISTENER_HPP

#include <functional>
#include <set>

#include "Vazteran/Core/Type.hpp"
#include "Vazteran/System/Scene.hpp"

namespace vzt
{
	enum class SystemEvent
	{
		Construct,
		Update,
		Destroy
	};

	template <class ComponentType>
	class Listener
	{
	  public:
		using ListenerCallback = std::function<void(Entity entity, SystemEvent eventType)>;

		Listener() = default;
		Listener(Ptr<const Scene> scene, ListenerCallback callback);

		Listener(const Listener<ComponentType>& other);
		Listener& operator=(const Listener<ComponentType>& other);

		Listener(Listener<ComponentType>&& other)            = default;
		Listener& operator=(Listener<ComponentType>&& other) = default;

		~Listener();

		void subscribe(SystemEvent eventType);
		void unsubscribe(SystemEvent eventType);

	  private:
		void onConstruct(entt::registry& registry, entt::entity entity);
		void onUpdate(entt::registry& registry, entt::entity entity);
		void onDestroy(entt::registry& registry, entt::entity entity);

		Ptr<const Scene>      m_scene;
		ListenerCallback      m_callback;
		std::set<SystemEvent> m_registeredEvent;
	};
} // namespace vzt

#include "Vazteran/System/Listener.inl"

#endif // VAZTERAN_LISTENER_HPP
