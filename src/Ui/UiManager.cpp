#include "Vazteran/Ui/UiManager.hpp"

namespace vzt
{
	namespace ui
	{
		void UiManager::setMainMenuBar(MainMenuBar mainMenuBar) { m_mainMenuBar = mainMenuBar; }

		void UiManager::draw() const
		{
			if (m_mainMenuBar)
				m_mainMenuBar->draw();
		}
	} // namespace ui
} // namespace vzt
