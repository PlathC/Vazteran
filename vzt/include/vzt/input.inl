#include "input.hpp"

namespace vzt
{
    inline void input::setClicked(Key key)
    {
        keyClicked.emplace(key);
        keyPressed.emplace(key);
    }

    inline void input::setReleased(Key key)
    {
        keyPressed.erase(key);
        keyReleased.emplace(key);
    }

    inline void input::reset()
    {
        mouseLeftClicked   = false;
        mouseRightClicked  = false;
        mouseMiddleClicked = false;

        windowResized      = false;
        deltaMousePosition = {};

        keyClicked.clear();
        keyReleased.clear();
    }

    inline bool input::isPressed(Key key) const { return static_cast<bool>(keyPressed.count(key)); }
    inline bool input::isClicked(Key key) const { return static_cast<bool>(keyClicked.count(key)); }
    inline bool input::isReleased(Key key) const { return static_cast<bool>(keyReleased.count(key)); }
} // namespace vzt
