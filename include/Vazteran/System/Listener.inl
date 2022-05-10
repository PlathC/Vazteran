#include "Vazteran/System/Listener.hpp"

namespace vzt
{
	template <class ComponentType>
	Listener<ComponentType>::Listener(Ptr<const Scene> scene, ListenerCallback callback)
	    : m_scene(std::move(scene)), m_callback(std::move(callback))
	{
	}

	template <class ComponentType>
	Listener<ComponentType>::Listener(const Listener<ComponentType>& other)
	    : m_scene(other.m_scene), m_callback(other.m_callback)
	{
		for (const SystemEvent registeredEvent : other.m_registeredEvent)
			subscribe(registeredEvent);
	}
	template <class ComponentType>
	Listener<ComponentType>& Listener<ComponentType>::operator=(const Listener<ComponentType>& listener)
	{
		m_scene    = other.m_scene;
		m_callback = other.m_callback;
		for (const SystemEvent registeredEvent : other.m_registeredEvent)
			subscribe(registeredEvent);

		return *this;
	}

	template <class ComponentType>
	Listener<ComponentType>::~Listener()
	{
		while (!m_registeredEvent.empty())
		{
			unsubscribe(*m_registeredEvent.begin());
		}
	}

	template <class ComponentType>
	void Listener<ComponentType>::subscribe(SystemEvent event)
	{
		entt::registry& registry = m_scene.m_registry;
		switch (event)
		{
		case SystemEvent::Construct:
			registry.on_construct<ComponentType>().connect<&Listener<ComponentType>::onConstruct>(this);
			break;
		case SystemEvent::Update:
			registry.on_update<ComponentType>().connect<&Listener<ComponentType>::onUpdate>(this);
			break;
		case SystemEvent::Destroy:
			registry.on_destroy<ComponentType>().connect<&Listener<ComponentType>::onDestroy>(this);
			break;
		}
		m_registeredEvent.emplace(event);
	}

	template <class ComponentType>
	void Listener<ComponentType>::unsubscribe(SystemEvent event)
	{
		entt::registry& registry = m_scene.m_registry;
		switch (event)
		{
		case SystemEvent::Construct:
			registry.on_construct<ComponentType>().disconnect<&Listener<ComponentType>::onConstruct>(this);
			break;
		case SystemEvent::Update:
			registry.on_update<ComponentType>().disconnect<&Listener<ComponentType>::onUpdate>(this);
			break;
		case SystemEvent::Destroy:
			registry.on_destroy<ComponentType>().disconnect<&Listener<ComponentType>::onDestroy>(this);
			break;
		}
		m_registeredEvent.erase(event);
	}

	template <class ComponentType>
	void Listener<ComponentType>::onConstruct(entt::registry& registry, entt::entity entity)
	{
		m_callback({registry, entity}, SystemEvent::Construct);
	}

	template <class ComponentType>
	void Listener<ComponentType>::onUpdate(entt::registry& registry, entt::entity entity)
	{
		m_callback({registry, entity}, SystemEvent::Update);
	}

	template <class ComponentType>
	void Listener<ComponentType>::onDestroy(entt::registry& registry, entt::entity entity)
	{
		m_callback({registry, entity}, SystemEvent::Destroy);
	}
} // namespace vzt
