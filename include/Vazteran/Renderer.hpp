#ifndef VAZTERAN_RENDERER_HPP
#define VAZTERAN_RENDERER_HPP

#include <vulkan/vulkan.h>

#include "Vazteran/Utils.hpp"
#include "Vazteran/Data/Camera.hpp"
#include "Vazteran/Vulkan/Buffer.hpp"
#include "Vazteran/Vulkan/CommandPool.hpp"
#include "Vazteran/Vulkan/Instance.hpp"
#include "Vazteran/Vulkan/SwapChain.hpp"
#include "Vazteran/Vulkan/RenderTarget.hpp"

namespace vzt {
    class GraphicPipeline;
    class ImageView;
    class LogicalDevice;
    class Model;
    class PhysicalDevice;

    class Renderer {
    public:
        Renderer(vzt::Instance* instance, VkSurfaceKHR surface, vzt::Size2D<int> size, vzt::Camera camera,
                 std::vector<vzt::Model*> models = {});

        void Draw();
        const vzt::LogicalDevice* Device() const { return m_logicalDevice.get(); }
        void FrameBufferResized(vzt::Size2D<int> newSize);

    private:
        constexpr static uint32_t MaxFramesInFlight = 2;

        VkSurfaceKHR m_surface;

        std::unique_ptr<vzt::PhysicalDevice> m_physicalDevice;
        std::unique_ptr<vzt::LogicalDevice> m_logicalDevice;
        std::unique_ptr<vzt::SwapChain> m_swapChain;
        vzt::CommandPool m_commandPool;

        // std::unique_ptr<vzt::Model> m_model;
        struct ModelRenderTarget {
            vzt::Model* model;
            vzt::RenderTarget vkTarget;
        };
        std::vector<ModelRenderTarget> m_targets;

        vzt::Camera m_camera;
    };
}

#endif //VAZTERAN_RENDERER_HPP
