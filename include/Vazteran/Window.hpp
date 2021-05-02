#ifndef VAZTERAN_WINDOW_HPP
#define VAZTERAN_WINDOW_HPP

#include <functional>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Vulkan/Buffer.hpp"
#include "Vazteran/Vulkan/Instance.hpp"

namespace vzt {
    class LogicalDevice;
    class GraphicPipeline;
    class Model;
    class PhysicalDevice;
    class SwapChain;
    class TextureImage;

    struct SurfaceHandler {
    public:
        SurfaceHandler(Instance* instance, VkSurfaceKHR surface);
        VkSurfaceKHR VkHandle() const { return m_surface; }
        ~SurfaceHandler();

    private:
        Instance* m_instance;
        VkSurfaceKHR m_surface;
    };

    using GLFWwindowPtr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

    class Window {
    public:
        Window(Instance* instance, std::string_view name, uint32_t width, uint32_t height);

        bool ShouldClose() { return glfwWindowShouldClose(m_window.get()); }
        void Draw();
        void FrameBufferResized();
        LogicalDevice* Device() { return m_logicalDevice.get(); }

        ~Window();
    private:
        uint32_t m_width;
        uint32_t m_height;


        GLFWwindowPtr m_window;
        Instance* m_instance;
        std::unique_ptr<SurfaceHandler> m_surface;
        std::unique_ptr<PhysicalDevice> m_physicalDevice;
        std::unique_ptr<LogicalDevice> m_logicalDevice;
        std::unique_ptr<SwapChain> m_swapChain;
        std::unique_ptr<VertexBuffer> m_vertexBuffer;
        std::unique_ptr<IndexBuffer> m_indexBuffer;
        Camera m_camera;
        std::unique_ptr<Model> m_model;
    };
}

#endif //VAZTERAN_WINDOW_HPP
