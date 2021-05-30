#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "Vazteran/Window.hpp"
#include "Vazteran/Vulkan/Instance.hpp"

namespace vzt {
    SurfaceHandler::SurfaceHandler(vzt::Instance* instance, VkSurfaceKHR surface) :
            m_instance(instance), m_surface(surface) {}

    SurfaceHandler::~SurfaceHandler() {
        vkDestroySurfaceKHR(m_instance->VkHandle(), m_surface, nullptr);
    }

    Window::Window(std::string_view name, const uint32_t width, const uint32_t height,
                   FrameBufferResizedCallback fbResizedCallback) :
            m_width(width), m_height(height), m_fbResizedCallback(std::move(fbResizedCallback)) {
        if(!glfwInit())
            throw std::runtime_error("Failed to load glfw.");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = GLFWwindowPtr(
                glfwCreateWindow(m_width, m_height, std::string(name).c_str(), nullptr, nullptr),
                [](GLFWwindow* window) {
                    glfwDestroyWindow(window);
                }
        );

        glfwSetWindowUserPointer(m_window.get(), this);
        glfwSetFramebufferSizeCallback(m_window.get(), [](GLFWwindow* window, int width, int height) {
            auto windowHandle = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            windowHandle->FrameBufferResized();
        });
    }

    void Window::FrameBufferResized() const {
        m_fbResizedCallback();
    }

    vzt::Size2D<int> Window::FrameBufferSize() const {
        int frameBufferWidth = 0, frameBufferHeight = 0;
        glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
        while (frameBufferWidth == 0 || frameBufferHeight == 0) {
            glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
            glfwWaitEvents();
        }
        return { frameBufferWidth, frameBufferHeight };
    }

    VkSurfaceKHR Window::Surface(vzt::Instance* instance) {
        if (!m_surface){
            VkSurfaceKHR surface;
            if (glfwCreateWindowSurface(instance->VkHandle(), m_window.get(), nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface!");
            }
            m_surface = std::make_unique<SurfaceHandler>(instance, surface);
        }

        return m_surface->VkHandle();
    }

    bool Window::Update() const {
        glfwPollEvents();

        return ShouldClose();
    }

    std::vector<const char*> Window::VkExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions;
        extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

        return extensions;
    }

    Window::~Window() = default;
}
