#include "Vazteran/Ui/Component.hpp"

namespace vzt
{
	namespace ui
	{
		Component::Component(std::string name) : m_name(std::move(name)) {}

		bool Component::ToggleEnable()
		{
			m_enable = !m_enable;
			return m_enable;
		};

		ActionComponent::ActionComponent(std::string name, Action action)
		    : Component(std::move(name)), m_action(std::move(action))
		{
		}
	} // namespace ui
} // namespace vzt
