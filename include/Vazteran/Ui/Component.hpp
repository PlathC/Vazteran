#ifndef VAZTERAN_UI_COMPONENT_HPP
#define VAZTERAN_UI_COMPONENT_HPP

#include <functional>
#include <string>
#include <string_view>

namespace vzt
{
	namespace ui
	{
		class Component
		{
		  public:
			Component(std::string name = "");
			virtual ~Component() = default;

			bool             isEnable() const { return m_enable; }
			std::string_view name() const { return m_name; }

			bool toggleEnable();

			virtual void draw() const = 0;

		  protected:
			std::string m_name;
			bool        m_enable = true;
		};

		using Action = std::function<void()>;
		class ActionComponent : public Component
		{
		  public:
			ActionComponent(
			    std::string name = "", Action action = []() {});

		  protected:
			virtual void execute() const { m_action(); };

		  private:
			Action m_action;
		};
	} // namespace ui
} // namespace vzt

#endif // VAZTERAN_UI_COMPONENT_HPP
