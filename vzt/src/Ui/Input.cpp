#include "vzt/Ui/Input.hpp"

namespace vzt
{
    void Input::reset()
    {
        mouseLeftClicked   = false;
        mouseRightClicked  = false;
        mouseMiddleClicked = false;

        windowResized      = false;
        deltaMousePosition = {};
    }
} // namespace vzt
