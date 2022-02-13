#include "Vazteran/Ui/UiManager.hpp"

namespace vzt
{
	namespace ui
	{
		void UiManager::setMainMenuBar(vzt::ui::MainMenuBar mainMenuBar) { m_mainMenuBar = mainMenuBar; }

		void UiManager::draw() const
		{
			if (m_mainMenuBar.has_value())
			{
				m_mainMenuBar.value().draw();
			}
		}
	} // namespace ui
} // namespace vzt
