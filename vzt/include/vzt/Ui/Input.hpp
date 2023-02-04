#ifndef VZT_INPUT_HPP
#define VZT_INPUT_HPP

#include <unordered_set>

#include "vzt/Core/Math.hpp"

namespace vzt
{
    enum class Key
    {
        Unknown,

        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Num0,

        Return,
        Escape,
        Backspace,
        Tab,
        Space,

        Minus,
        Equals,
        LeftBracket,
        RightBracket,
        BackSlash,

        SemiColon,
        Apostrophe,

        Comma,
        Slash,

        CapsLock,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        PrintScreen,
        ScrollLock,
        Pause,
        Insert,
        Home,
        PageUp,
        Delete,
        End,
        PageDown,
        Right,
        Left,
        Down,
        Up,

        NumLockClear,
        KpDivide,
        KpMultiply,
        KpMinus,
        KpPlus,
        KpEnter,
        Kp1,
        Kp2,
        Kp3,
        Kp4,
        Kp5,
        Kp6,
        Kp7,
        Kp8,
        Kp9,
        Kp0,

        LCtrl,
        LShift,
        LAlt,
        LGui,
        RCtrl,
        RShift,
        RAlt,
        RGui,
    };

    struct Input
    {
        Extent2D windowSize;
        bool     windowResized = false;

        uint64_t time;
        float    deltaTime;

        Vec2i mousePosition;
        Vec2i deltaMousePosition;

        bool mouseLeftClicked = false;
        bool mouseLeftPressed = false;

        bool mouseRightClicked = false;
        bool mouseRightPressed = false;

        bool mouseMiddleClicked = false;
        bool mouseMiddlePressed = false;

        std::unordered_set<Key> keyPressed;
        std::unordered_set<Key> keyClicked;
        std::unordered_set<Key> keyReleased;

        inline void setClicked(Key key);
        inline void setReleased(Key key);
        inline void reset();

        inline bool isPressed(Key key) const;
        inline bool isClicked(Key key) const;
        inline bool isReleased(Key key) const;
    };
} // namespace vzt

#include "vzt/Ui/Input.inl"

#endif // VZT_INPUT_HPP
