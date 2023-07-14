#ifndef VZT_WINDOW_HPP
#define VZT_WINDOW_HPP

#include <atomic>
#include <string>

#include "vzt/Ui/Input.hpp"
#include "vzt/Vulkan/Instance.hpp"

union SDL_Event;
struct SDL_Window;
namespace vzt
{
    class Window
    {
      public:
        Window(std::string title, uint32_t width = 1280, uint32_t height = 720);

        Window(const Window&)            = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;

        ~Window();

        inline std::string_view getTitle() const;
        inline uint32_t         getWidth() const;
        inline uint32_t         getHeight() const;
        inline Extent2D         getExtent() const;
        inline const Input&     getInputs() const;
        inline SDL_Window*      getHandle() const;

        using EventCallback = std::function<void(SDL_Event*)>;
        inline void setEventCallback(EventCallback eventCallback);

        bool update();

        friend Instance;
        friend Surface;

      private:
        InstanceBuilder getConfiguration(InstanceBuilder configuration = {}) const;

        static std::atomic_size_t m_instanceCount;

        std::string m_title;
        uint32_t    m_width;
        uint32_t    m_height;

        SDL_Window* m_handle = nullptr;

        Input    m_inputs;
        uint64_t m_lastTimeStep = 0;

        EventCallback m_eventCallback = {};
    };

} // namespace vzt

#include "vzt/Window.inl"

#endif // VZT_WINDOW_HPP
