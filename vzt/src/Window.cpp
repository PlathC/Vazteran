#include "vzt/Window.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>

#include "vzt/Core/Logger.hpp"

namespace vzt
{
    std::atomic_size_t Window::m_instanceCount = 0;

    Window::Window(std::string title, const uint32_t width, const uint32_t height)
        : m_title(std::move(title)), m_width(width), m_height(height)
    {
        m_instanceCount++;
        if (m_instanceCount == 1 && SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
            logger::error("[SDL] {}", SDL_GetError());

        m_handle =
            SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int>(width),
                             static_cast<int>(height),
                             SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

        if (!m_handle)
            logger::error(SDL_GetError());
    }

    Window::Window(Window&& other) noexcept
    {
        std::swap(m_title, other.m_title);
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
        std::swap(m_handle, other.m_handle);
        std::swap(m_inputs, other.m_inputs);
    }

    Window& Window::operator=(Window&& other) noexcept
    {
        std::swap(m_title, other.m_title);
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
        std::swap(m_handle, other.m_handle);
        std::swap(m_inputs, other.m_inputs);

        return *this;
    }

    Window::~Window()
    {
        if (!m_handle)
            return;

        SDL_DestroyWindow(m_handle);

        m_instanceCount--;
        if (m_instanceCount == 0)
            SDL_Quit();
    }

    static Key toKey(SDL_Scancode scanCode);

    bool Window::update()
    {
        m_inputs.reset();

        // Based on ImGui
        // https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_sdl.cpp#L557
        static const uint64_t sdlFrequency = SDL_GetPerformanceFrequency();
        const uint64_t        now          = SDL_GetPerformanceCounter();
        m_inputs.deltaTime = static_cast<float>(static_cast<double>(now - m_lastTimeStep) / sdlFrequency);
        m_lastTimeStep     = now;
        m_inputs.time      = SDL_GetTicks64();

        bool      closing = false;
        SDL_Event windowEvent;
        while (SDL_PollEvent(&windowEvent))
        {
            if (m_eventCallback)
                m_eventCallback(&windowEvent);

            switch (windowEvent.type)
            {
            case SDL_QUIT: closing = true; break;
            case SDL_MOUSEMOTION:
                m_inputs.deltaMousePosition.x = windowEvent.motion.xrel;
                m_inputs.deltaMousePosition.y = windowEvent.motion.yrel;
                m_inputs.mousePosition.x      = windowEvent.motion.x;
                m_inputs.mousePosition.y      = windowEvent.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (windowEvent.button.button)
                {
                case SDL_BUTTON_LEFT: {
                    m_inputs.mouseLeftPressed = true;
                    m_inputs.mouseLeftClicked = true;
                    break;
                }
                case SDL_BUTTON_RIGHT:
                    m_inputs.mouseRightPressed = true;
                    m_inputs.mouseRightClicked = true;
                    break;
                case SDL_BUTTON_MIDDLE:
                    m_inputs.mouseMiddlePressed = true;
                    m_inputs.mouseMiddleClicked = true;
                    break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (windowEvent.button.button)
                {
                case SDL_BUTTON_LEFT: m_inputs.mouseLeftPressed = false; break;
                case SDL_BUTTON_RIGHT: m_inputs.mouseRightPressed = false; break;
                case SDL_BUTTON_MIDDLE: m_inputs.mouseMiddlePressed = false; break;
                }
                break;
            case SDL_WINDOWEVENT:
                if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    m_width  = uint32_t(windowEvent.window.data1);
                    m_height = uint32_t(windowEvent.window.data2);

                    m_inputs.windowSize    = {m_width, m_height};
                    m_inputs.windowResized = true;
                }
                break;

            case SDL_KEYDOWN: {
                Key key = toKey(windowEvent.key.keysym.scancode);
                m_inputs.setClicked(key);
                break;
            }
            case SDL_KEYUP: {
                Key key = toKey(windowEvent.key.keysym.scancode);
                m_inputs.setReleased(key);
                break;
            }
            }
        }

        return !closing;
    }

    InstanceBuilder Window::getConfiguration(InstanceBuilder configuration) const
    {
        uint32_t count = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(m_handle, &count, nullptr))
            logger::error("[SDL] {}", SDL_GetError());

        const std::size_t baseExtensionNb = configuration.extensions.size();
        configuration.extensions.resize(baseExtensionNb + count);

        if (!SDL_Vulkan_GetInstanceExtensions(m_handle, &count, configuration.extensions.data() + baseExtensionNb))
            logger::error("[SDL] {}", SDL_GetError());

        return configuration;
    }

    static Key toKey(SDL_Scancode scanCode)
    {
        switch (scanCode)
        {
        case SDL_SCANCODE_UNKNOWN: return Key::Unknown;
        case SDL_SCANCODE_A: return Key::A;
        case SDL_SCANCODE_B: return Key::B;
        case SDL_SCANCODE_C: return Key::C;
        case SDL_SCANCODE_D: return Key::D;
        case SDL_SCANCODE_E: return Key::E;
        case SDL_SCANCODE_F: return Key::F;
        case SDL_SCANCODE_G: return Key::G;
        case SDL_SCANCODE_H: return Key::H;
        case SDL_SCANCODE_I: return Key::I;
        case SDL_SCANCODE_J: return Key::J;
        case SDL_SCANCODE_K: return Key::K;
        case SDL_SCANCODE_L: return Key::L;
        case SDL_SCANCODE_M: return Key::M;
        case SDL_SCANCODE_N: return Key::N;
        case SDL_SCANCODE_O: return Key::O;
        case SDL_SCANCODE_P: return Key::P;
        case SDL_SCANCODE_Q: return Key::Q;
        case SDL_SCANCODE_R: return Key::R;
        case SDL_SCANCODE_S: return Key::S;
        case SDL_SCANCODE_T: return Key::T;
        case SDL_SCANCODE_U: return Key::U;
        case SDL_SCANCODE_V: return Key::V;
        case SDL_SCANCODE_W: return Key::W;
        case SDL_SCANCODE_X: return Key::X;
        case SDL_SCANCODE_Y: return Key::Y;
        case SDL_SCANCODE_Z: return Key::Z;
        case SDL_SCANCODE_1: return Key::Num1;
        case SDL_SCANCODE_2: return Key::Num2;
        case SDL_SCANCODE_3: return Key::Num3;
        case SDL_SCANCODE_4: return Key::Num4;
        case SDL_SCANCODE_5: return Key::Num5;
        case SDL_SCANCODE_6: return Key::Num6;
        case SDL_SCANCODE_7: return Key::Num7;
        case SDL_SCANCODE_8: return Key::Num8;
        case SDL_SCANCODE_9: return Key::Num9;
        case SDL_SCANCODE_0: return Key::Num0;
        case SDL_SCANCODE_RETURN: return Key::Return;
        case SDL_SCANCODE_ESCAPE: return Key::Escape;
        case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
        case SDL_SCANCODE_TAB: return Key::Tab;
        case SDL_SCANCODE_SPACE: return Key::Space;
        case SDL_SCANCODE_MINUS: return Key::Minus;
        case SDL_SCANCODE_EQUALS: return Key::Equals;
        case SDL_SCANCODE_LEFTBRACKET: return Key::LeftBracket;
        case SDL_SCANCODE_RIGHTBRACKET: return Key::RightBracket;
        case SDL_SCANCODE_BACKSLASH: return Key::BackSlash;
        case SDL_SCANCODE_SEMICOLON: return Key::SemiColon;
        case SDL_SCANCODE_APOSTROPHE: return Key::Apostrophe;
        case SDL_SCANCODE_COMMA: return Key::Comma;
        case SDL_SCANCODE_SLASH: return Key::Slash;
        case SDL_SCANCODE_CAPSLOCK: return Key::CapsLock;
        case SDL_SCANCODE_F1: return Key::F1;
        case SDL_SCANCODE_F2: return Key::F2;
        case SDL_SCANCODE_F3: return Key::F3;
        case SDL_SCANCODE_F4: return Key::F4;
        case SDL_SCANCODE_F5: return Key::F5;
        case SDL_SCANCODE_F6: return Key::F6;
        case SDL_SCANCODE_F7: return Key::F7;
        case SDL_SCANCODE_F8: return Key::F8;
        case SDL_SCANCODE_F9: return Key::F9;
        case SDL_SCANCODE_F10: return Key::F10;
        case SDL_SCANCODE_F11: return Key::F11;
        case SDL_SCANCODE_F12: return Key::F12;
        case SDL_SCANCODE_PRINTSCREEN: return Key::PrintScreen;
        case SDL_SCANCODE_SCROLLLOCK: return Key::ScrollLock;
        case SDL_SCANCODE_PAUSE: return Key::Pause;
        case SDL_SCANCODE_INSERT: return Key::Insert;
        case SDL_SCANCODE_HOME: return Key::Home;
        case SDL_SCANCODE_PAGEUP: return Key::PageUp;
        case SDL_SCANCODE_DELETE: return Key::Delete;
        case SDL_SCANCODE_END: return Key::End;
        case SDL_SCANCODE_PAGEDOWN: return Key::PageDown;
        case SDL_SCANCODE_RIGHT: return Key::Right;
        case SDL_SCANCODE_LEFT: return Key::Left;
        case SDL_SCANCODE_DOWN: return Key::Down;
        case SDL_SCANCODE_UP: return Key::Up;
        case SDL_SCANCODE_NUMLOCKCLEAR: return Key::NumLockClear;
        case SDL_SCANCODE_KP_DIVIDE: return Key::KpDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return Key::KpMultiply;
        case SDL_SCANCODE_KP_MINUS: return Key::KpMinus;
        case SDL_SCANCODE_KP_PLUS: return Key::KpPlus;
        case SDL_SCANCODE_KP_ENTER: return Key::KpEnter;
        case SDL_SCANCODE_KP_1: return Key::Kp1;
        case SDL_SCANCODE_KP_2: return Key::Kp2;
        case SDL_SCANCODE_KP_3: return Key::Kp3;
        case SDL_SCANCODE_KP_4: return Key::Kp4;
        case SDL_SCANCODE_KP_5: return Key::Kp5;
        case SDL_SCANCODE_KP_6: return Key::Kp6;
        case SDL_SCANCODE_KP_7: return Key::Kp7;
        case SDL_SCANCODE_KP_8: return Key::Kp8;
        case SDL_SCANCODE_KP_9: return Key::Kp9;
        case SDL_SCANCODE_KP_0: return Key::Kp0;
        case SDL_SCANCODE_LCTRL: return Key::LCtrl;
        case SDL_SCANCODE_LSHIFT: return Key::LShift;
        case SDL_SCANCODE_LALT: return Key::LAlt;
        case SDL_SCANCODE_LGUI: return Key::LGui;
        case SDL_SCANCODE_RCTRL: return Key::RCtrl;
        case SDL_SCANCODE_RSHIFT: return Key::RShift;
        case SDL_SCANCODE_RALT: return Key::RAlt;
        case SDL_SCANCODE_RGUI: return Key::RGui;
        }
        return Key::Unknown;
    }

} // namespace vzt
