#include "Vazteran/Ui/MainMenuBar.hpp"

namespace vzt
{
	namespace ui
	{
		MainMenuItem::MainMenuItem(std::string name, vzt::ui::MenuItemAction action)
		    : ActionComponent(std::move(name), std::move(action))
		{
		}

		void MainMenuItem::draw() const
		{
			if (m_enable && ImGui::MenuItem(m_name.c_str()))
			{
				execute();
			}
		}

		MainMenuField::MainMenuField(std::string name) : Component(std::move(name)) {}

		void MainMenuField::draw() const
		{
			if (ImGui::BeginMenu(m_name.c_str()))
			{
				for (std::size_t i = 0; i < m_items.size(); i++)
				{
					m_items[i].draw();
				}

				ImGui::EndMenu();
			}
		}

		void MainMenuField::addItem(vzt::ui::MainMenuItem item) { m_items.emplace_back(std::move(item)); }

		MainMenuBar::MainMenuBar() : vzt::ui::Component() {}

		void MainMenuBar::draw() const
		{
			if (ImGui::BeginMainMenuBar())
			{
				for (std::size_t i = 0; i < m_menus.size(); i++)
				{
					m_menus[i].draw();
				}
				ImGui::EndMainMenuBar();
			}
		}

		void MainMenuBar::addMenu(vzt::ui::MainMenuField menuItem) { m_menus.emplace_back(std::move(menuItem)); }

	} // namespace ui
} // namespace vzt
