#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "Vazteran/Window.hpp"
#include "Vazteran/Vulkan/DeviceManager.hpp"
#include "Vazteran/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    SurfaceHandler::SurfaceHandler(Instance* instance, VkSurfaceKHR surface) :
            m_instance(instance), m_surface(surface) {}

    SurfaceHandler::~SurfaceHandler() {
        vkDestroySurfaceKHR(m_instance->Get(), m_surface, nullptr);
    }

    Window::Window(Instance* instance, std::string_view name, const uint32_t width, const uint32_t height) :
            m_instance(instance), m_width(width), m_height(height) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_window = GLFWwindowPtr(
                glfwCreateWindow(m_width, m_height, std::string(name).c_str(), nullptr, nullptr),
                [](GLFWwindow* window) {
                    glfwDestroyWindow(window);
                }
        );

        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(m_instance->Get(), m_window.get(), nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        m_surface = std::make_unique<SurfaceHandler>(m_instance, surface);
        m_device = std::make_unique<vzt::DeviceManager>(m_instance, surface);

        int frameBufferWidth, frameBufferHeight;
        glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
        m_swapChain = std::make_unique<vzt::SwapChain>(m_device.get(), surface, frameBufferWidth, frameBufferHeight);
    }

    void Window::Draw() {
        if(m_swapChain->DrawFrame()) {
            vkDeviceWaitIdle(m_device->LogicalDevice());
            int frameBufferWidth, frameBufferHeight;
            glfwGetFramebufferSize(m_window.get(), &frameBufferWidth, &frameBufferHeight);
            // Recreate swapchain

        }
    }

    Window::~Window() {

    }
}
