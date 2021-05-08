#ifndef VAZTERAN_RENDERER_HPP
#define VAZTERAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Utils.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Vulkan/Buffer.hpp"
#include "Vazteran/Vulkan/Instance.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"

namespace vzt {
    class GraphicPipeline;
    class ImageView;
    class LogicalDevice;
    class Model;
    class PhysicalDevice;

    class Renderer {
    public:
        Renderer(vzt::Instance* instance, VkSurfaceKHR surface, vzt::Size2D size,
                std::unique_ptr<Model> model, Camera camera);

        void Draw();
        vzt::LogicalDevice* Device() const { return m_logicalDevice.get(); }
        void FrameBufferResized(vzt::Size2D newSize);

    private:
        vzt::Instance* m_instance;
        VkSurfaceKHR m_surface;

        std::unique_ptr<vzt::PhysicalDevice> m_physicalDevice;
        std::unique_ptr<vzt::LogicalDevice> m_logicalDevice;
        std::unique_ptr<vzt::SwapChain> m_swapChain;
        std::unique_ptr<VertexBuffer> m_vertexBuffer;
        std::unique_ptr<IndexBuffer> m_indexBuffer;

        std::unique_ptr<vzt::Model> m_model;
        vzt::Camera m_camera;
    };
}

#endif //VAZTERAN_RENDERER_HPP
