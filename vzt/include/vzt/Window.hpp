#ifndef VZT_WINDOW_HPP
#define VZT_WINDOW_HPP

#include <atomic>

#include "vzt/Instance.hpp"

struct SDL_Window;
namespace vzt
{
    class Window
    {
      public:
        Window(std::string title, const uint32_t width = 1280, const uint32_t height = 720);

        Window(const Window&)            = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;

        ~Window();

        inline std::string_view getTitle() const;
        inline uint32_t         getWith() const;
        inline uint32_t         getHeight() const;
        inline SDL_Window*      getHandle() const;

        Configuration getConfiguration(Configuration configuration = {});

        friend Instance;

      private:
        void assign(const Instance& instance);

        static std::atomic_size_t m_instanceCount;

        std::string m_title;
        uint32_t    m_width;
        uint32_t    m_height;

        SDL_Window*     m_handle   = nullptr;
        const Instance* m_instance = nullptr;
        VkSurfaceKHR    m_surface  = VK_NULL_HANDLE;
    };

} // namespace vzt

#include "vzt/Window.inl"

#endif // VZT_WINDOW_HPP
