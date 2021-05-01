#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "Vazteran/Window.hpp"
#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/LogicalDevice.hpp"
#include "Vazteran/Vulkan/PhysicalDevice.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    SurfaceHandler::SurfaceHandler(Instance* instance, VkSurfaceKHR surface) :
            m_instance(instance), m_surface(surface) {}

    SurfaceHandler::~SurfaceHandler() {
        vkDestroySurfaceKHR(m_instance->VkHandle(), m_surface, nullptr);
    }

    Window::Window(Instance* instance, std::string_view name, const uint32_t width, const uint32_t height) :
            m_instance(instance), m_width(width), m_height(height) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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

        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(m_instance->VkHandle(), m_window.get(), nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        m_surface = std::make_unique<SurfaceHandler>(m_instance, surface);
        m_physicalDevice = std::make_unique<PhysicalDevice>(m_instance, surface);
        m_logicalDevice = std::make_unique<LogicalDevice>(m_instance, m_physicalDevice.get(), surface);

        int frameBufferWidth, frameBufferHeight;
        glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
        m_swapChain = std::make_unique<vzt::SwapChain>(m_logicalDevice.get(), surface, frameBufferWidth, frameBufferHeight);
    }

    void Window::Draw() {
        if(m_swapChain->DrawFrame()) {
            vkDeviceWaitIdle(m_logicalDevice->VkHandle());
            int frameBufferWidth, frameBufferHeight;
            glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
            while (frameBufferWidth == 0 || frameBufferHeight == 0) {
                glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
                glfwWaitEvents();
            }

            m_swapChain->Recreate(m_surface->VkHandle(), frameBufferWidth, frameBufferHeight);
        }
    }

    void Window::FrameBufferResized() {
        m_swapChain->FrameBufferResized();
    }

    Window::~Window() {

    }
}
