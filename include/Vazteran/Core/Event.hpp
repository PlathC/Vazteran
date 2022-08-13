#ifndef VAZTERAN_EVENT_HPP
#define VAZTERAN_EVENT_HPP

#include <entt/signal/delegate.hpp>
#include <entt/signal/dispatcher.hpp>
#include <entt/signal/sigh.hpp>

namespace vzt
{
	class ConnectionHolder;

	using Connection = entt::connection;

	template <class EventType>
	using Listener = entt::delegate<void(EventType)>;

	class ConnectionHolder;
	class Provider
	{
	  public:
		Provider()          = default;
		virtual ~Provider() = default;

		template <class EventType, auto Candidate, typename... Type>
		Connection subscribe(Type&&... valueOrInstance);

	  protected:
		entt::dispatcher m_dispatcher{};
	};

	class ConnectionHolder
	{
	  public:
		ConnectionHolder() = default;

		ConnectionHolder(const ConnectionHolder& other)            = delete;
		ConnectionHolder& operator=(const ConnectionHolder& other) = delete;

		ConnectionHolder(ConnectionHolder&& other)            = default;
		ConnectionHolder& operator=(ConnectionHolder&& other) = default;

		virtual ~ConnectionHolder();

		template <class EventType, auto Candidate, typename... Type>
		void subscribe(Provider& provider, Type&&... valueOrInstance);

		void unsubscribe();

		friend Provider;

	  private:
		std::vector<Connection> m_connections;
	};

	template <class EventType>
	struct DynamicListener
	{
	  public:
		using Callback = std::function<void(const EventType&)>;

		DynamicListener(Provider& provider, Callback callback);
		~DynamicListener() = default;

		void execute(const EventType& e);

	  private:
		Callback   m_callback;
		Connection m_connection;
	};
} // namespace vzt

#include "Vazteran/Core/Event.inl"

#endif // VAZTERAN_EVENT_HPP
