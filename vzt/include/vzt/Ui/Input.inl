#include "vzt/Ui/Input.hpp"

namespace vzt
{
    inline void Input::setClicked(Key key)
    {
        keyClicked.emplace(key);
        keyPressed.emplace(key);
    }

    inline void Input::setReleased(Key key)
    {
        keyPressed.erase(key);
        keyReleased.emplace(key);
    }

    inline void Input::reset()
    {
        mouseLeftClicked   = false;
        mouseRightClicked  = false;
        mouseMiddleClicked = false;

        windowResized      = false;
        deltaMousePosition = {};

        keyClicked.clear();
        keyReleased.clear();
    }

    inline bool Input::isPressed(Key key) const { return static_cast<bool>(keyPressed.count(key)); }
    inline bool Input::isClicked(Key key) const { return static_cast<bool>(keyClicked.count(key)); }
    inline bool Input::isReleased(Key key) const { return static_cast<bool>(keyReleased.count(key)); }
} // namespace vzt
