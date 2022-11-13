#include "vzt/Window.hpp"

namespace vzt
{
    inline std::string_view Window::getTitle() const { return m_title; }
    inline uint32_t         Window::getWith() const { return m_width; }
    inline uint32_t         Window::getHeight() const { return m_height; }
    inline Extent2D         Window::getExtent() const { return {m_width, m_height}; }
    inline SDL_Window*      Window::getHandle() const { return m_handle; }
    inline const Input&     Window::getInputs() const { return m_inputs; }
} // namespace vzt
