#ifndef VAZTERAN_UI_MAINMENUBAR_HPP
#define VAZTERAN_UI_MAINMENUBAR_HPP

#include <functional>
#include <vector>

#include <imgui.h>

#include "Vazteran/Ui/Component.hpp"

namespace vzt
{
	namespace ui
	{
		using MenuItemAction = vzt::ui::Action;
		class MainMenuItem : public vzt::ui::ActionComponent
		{
		  public:
			MainMenuItem(
			    std::string name = "", vzt::ui::MenuItemAction action = []() {});
			virtual ~MainMenuItem() = default;

			virtual void draw() const;
		};

		class MainMenuField : public vzt::ui::Component
		{
		  public:
			MainMenuField(std::string name = "");
			virtual ~MainMenuField() = default;

			virtual void draw() const;

			void addItem(vzt::ui::MainMenuItem item);

		  private:
			std::vector<vzt::ui::MainMenuItem> m_items;
		};

		class MainMenuBar : public Component
		{
		  public:
			MainMenuBar();
			virtual ~MainMenuBar() = default;

			virtual void draw() const;

			void addMenu(vzt::ui::MainMenuField menuItem);

		  private:
			std::vector<vzt::ui::MainMenuField> m_menus;
		};
	} // namespace ui
} // namespace vzt

#endif // VAZTERAN_UI_MAINMENUBAR_HPP
