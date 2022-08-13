#include "Vazteran/Core/Event.hpp"

namespace vzt
{
	ConnectionHolder::~ConnectionHolder() { unsubscribe(); }

	void ConnectionHolder::unsubscribe()
	{
		for (auto& connection : m_connections)
			connection.release();
	}
} // namespace vzt
