#ifndef VZT_INPUT_HPP
#define VZT_INPUT_HPP

#include "vzt/Core/Math.hpp"

namespace vzt
{
    struct Input
    {
        Extent2D windowSize;
        bool     windowResized = false;

        float deltaTime;

        Vec2i mousePosition;
        Vec2i deltaMousePosition;

        bool mouseLeftClicked = false;
        bool mouseLeftPressed = false;

        bool mouseRightClicked = false;
        bool mouseRightPressed = false;

        bool mouseMiddleClicked = false;
        bool mouseMiddlePressed = false;

        void reset();
    };
} // namespace vzt

#endif // VZT_INPUT_HPP
