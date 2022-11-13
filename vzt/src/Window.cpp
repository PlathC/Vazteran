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
        std::swap(m_instance, other.m_instance);
        std::swap(m_surface, other.m_surface);
        std::swap(m_inputs, other.m_inputs);
    }

    Window& Window::operator=(Window&& other) noexcept
    {
        std::swap(m_title, other.m_title);
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
        std::swap(m_handle, other.m_handle);
        std::swap(m_instance, other.m_instance);
        std::swap(m_surface, other.m_surface);
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

    bool Window::update()
    {
        m_inputs.reset();

        // Based on ImGui
        // https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_sdl.cpp#L557
        static const uint64_t sdlFrequency = SDL_GetPerformanceFrequency();
        const uint64_t        now          = SDL_GetPerformanceCounter();
        m_inputs.deltaTime = static_cast<float>(static_cast<double>(now - m_lastTimeStep) / sdlFrequency);
        m_lastTimeStep     = now;

        bool      closing = false;
        SDL_Event windowEvent;
        while (SDL_PollEvent(&windowEvent))
        {
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
                    m_width  = windowEvent.window.data1;
                    m_height = windowEvent.window.data2;

                    m_inputs.windowSize    = {m_width, m_height};
                    m_inputs.windowResized = true;
                }
                break;
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

} // namespace vzt
