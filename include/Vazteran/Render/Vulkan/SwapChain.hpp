//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_SWAPCHAIN_HPP
#define VAZTERAN_SWAPCHAIN_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include "Vazteran/Render/Vulkan/PhysicalDevice.hpp"
#include "Vazteran/Render/Vulkan/ImageView.hpp"
#include "Vazteran/Render/Vulkan/GraphicPipeline.hpp"
#include "Vazteran/Render/Vulkan/RenderPass.hpp"

namespace vzt
{
    class Surface;
    class SwapChain
    {
    public:
        SwapChain(std::shared_ptr<PhysicalDevice> physicalDevice, Surface* surface, int width, int height);
        ~SwapChain();
    private:
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height) const;
    private:
        std::shared_ptr<PhysicalDevice> m_physicalDevice;

        VkSwapchainKHR m_swapChain;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
        std::vector<VkImage> m_swapChainImages;
        std::vector<ImageView> m_swapChainImageViews;

        std::unique_ptr<GraphicPipeline> m_graphicPipeline;
        std::unique_ptr<RenderPass> m_renderPass;
    };
}


#endif //VAZTERAN_SWAPCHAIN_HPP
