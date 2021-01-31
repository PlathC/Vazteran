//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_RENDERPASS_HPP
#define VAZTERAN_RENDERPASS_HPP

#include <memory>

#include "Vazteran/Render/Vulkan/PhysicalDevice.hpp"

namespace vzt
{
    class RenderPass
    {
    public:
        RenderPass(std::shared_ptr<PhysicalDevice> physicalDevice, VkFormat swapChainImageFormat);
        ~RenderPass();
    private:
        std::shared_ptr<PhysicalDevice> m_physicalDevice;

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
    };
}


#endif //VAZTERAN_RENDERPASS_HPP
