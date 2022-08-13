#include "Vazteran/Core/Event.hpp"

#include <functional>

namespace vzt
{
	template <class EventType, auto Candidate, typename... Type>
	Connection Provider::subscribe(Type&&... valueOrInstance)
	{
		return m_dispatcher.sink<EventType>().connect<Candidate, Type...>(valueOrInstance...);
	}

	template <class EventType, auto Candidate, typename... Type>
	void ConnectionHolder::subscribe(Provider& provider, Type&&... valueOrInstance)
	{
		m_connections.emplace_back(
		    provider.template subscribe<EventType, Candidate, Type...>(std::forward<Type&&...>(valueOrInstance...)));
	}

	template <class EventType>
	DynamicListener<EventType>::DynamicListener(Provider& provider, Callback callback) : m_callback(std::move(callback))
	{
		m_connection = provider.subscribe<EventType, &DynamicListener::execute>(*this);
	}

	template <class EventType>
	void DynamicListener<EventType>::execute(const EventType& e)
	{
		m_callback(e);
	}
} // namespace vzt
