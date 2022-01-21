#include "Vazteran/Ui/UiManager.hpp"

namespace vzt
{
	namespace ui
	{
		void UiManager::SetMainMenuBar(vzt::ui::MainMenuBar mainMenuBar) { m_mainMenuBar = mainMenuBar; }

		void UiManager::Draw() const
		{
			if (m_mainMenuBar.has_value())
			{
				m_mainMenuBar.value().Draw();
			}
		}
	} // namespace ui
} // namespace vzt
