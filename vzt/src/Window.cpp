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
    }

    Window& Window::operator=(Window&& other) noexcept
    {
        std::swap(m_title, other.m_title);
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
        std::swap(m_handle, other.m_handle);
        std::swap(m_instance, other.m_instance);
        std::swap(m_surface, other.m_surface);

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
        bool      closing = false;
        SDL_Event windowEvent;
        while (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT)
                closing = true;
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
