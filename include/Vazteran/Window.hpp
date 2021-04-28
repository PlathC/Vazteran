#ifndef VAZTERAN_WINDOW_HPP
#define VAZTERAN_WINDOW_HPP

#include <functional>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vazteran/Vulkan/DeviceManager.hpp"
#include "Vazteran/Vulkan/Instance.hpp"

namespace vzt {
    class Window {
    public:
        Window(Instance* instance, std::string_view name, uint32_t width, uint32_t height);
        bool ShouldClose() { return glfwWindowShouldClose(m_window.get()); }
        ~Window() { vkDestroySurfaceKHR(m_instance->Get(), surface, nullptr); }
    private:
        Instance* m_instance;
        std::unique_ptr<vzt::DeviceManager> m_device;

        const uint32_t m_width;
        const uint32_t m_height;
        std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>> m_window;

        VkSurfaceKHR surface;
    };
}

#endif //VAZTERAN_WINDOW_HPP
