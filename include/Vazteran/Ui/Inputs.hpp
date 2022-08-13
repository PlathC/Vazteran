#ifndef VAZTERAN_INPUTS_HPP
#define VAZTERAN_INPUTS_HPP

#include <map>
#include <optional>
#include <unordered_set>

#include "Vazteran/Core/Logger.hpp"
#include "Vazteran/Ui/InputTypes.hpp"

namespace vzt
{
	template <class Actionable>
	class ActionableList
	{
	  public:
		void set(Actionable actionable, KeyAction action)
		{
			if (m_actions.find(actionable) != m_actions.end())
			{
				KeyAction& current = m_actions[actionable];
				current            = action != KeyAction::Release ? KeyAction::Press : KeyAction::Release;
			}
			else
			{
				m_actions[actionable] = action;
			}
		}

		void set(const std::map<Actionable, KeyAction>& currentActions)
		{
			for (auto pastAction = m_actions.begin(), newAction = currentActions.begin();
			     pastAction != m_actions.end() && newAction != currentActions.end();)
			{
				if (pastAction->first == newAction->first)
				{
					pastAction->second =
					    newAction->second != KeyAction::Release ? KeyAction::Press : KeyAction::Release;
					pastAction++;
					newAction++;
				}
				else if (pastAction.first < newAction.first)
				{
					m_actions.erase(pastAction);
				}
				else
				{
					m_actions.emplace(*newAction);
				}
			}
		}

		std::optional<KeyAction> operator[](Actionable actionable) const
		{
			if (m_actions.find(actionable) != m_actions.end())
				return m_actions[actionable];
			return {};
		}

	  private:
		mutable std::map<Actionable, KeyAction> m_actions;
	};

	struct Inputs
	{
		bool get(MouseButton button, KeyAction action = KeyAction::Press) const
		{
			const auto reported = mouseButtons[button];
			return reported ? *reported == action : false;
		}

		bool get(KeyCode button, KeyAction action = KeyAction::Press) const
		{
			const auto reported = keys[button];
			return reported ? *reported == action : false;
		}

		bool get(KeyModifier modifier) const { return (modifiers & modifier) == modifier; }

		void updateMousePosition(Vec2 position)
		{
			if (m_firstUpdate)
			{
				m_firstUpdate = false;
				mousePosition = position;
				return;
			}

			deltaMousePosition = position - mousePosition;
			mousePosition      = position;
		}

		void reset() { deltaMousePosition = {}; }

		ActionableList<MouseButton> mouseButtons;
		ActionableList<KeyCode>     keys;
		KeyModifier                 modifiers;

		Vec2 mousePosition;
		Vec2 deltaMousePosition;

	  private:
		bool m_firstUpdate = true;
	};
} // namespace vzt

#endif // VAZTERAN_INPUTS_HPP
