#ifndef VAZTERAN_UI_UIMANAGER_HPP
#define VAZTERAN_UI_UIMANAGER_HPP

#include <optional>

#include "Vazteran/Ui/Component.hpp"
#include "Vazteran/Ui/MainMenuBar.hpp"

namespace vzt
{
	namespace ui
	{
		class UiManager
		{
		  public:
			UiManager()          = default;
			virtual ~UiManager() = default;

			void setMainMenuBar(MainMenuBar mainMenuBar);

			void draw() const;

		  private:
			std::optional<MainMenuBar> m_mainMenuBar;
		};
	} // namespace ui
} // namespace vzt

#endif // VAZTERAN_UI_UIMANAGER_HPP
