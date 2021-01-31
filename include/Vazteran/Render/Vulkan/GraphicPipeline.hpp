//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_GRAPHICPIPELINE_HPP
#define VAZTERAN_GRAPHICPIPELINE_HPP

#include <memory>
#include <vector>

#include "Vazteran/Render/Vulkan/PhysicalDevice.hpp"

namespace vzt
{
    static VkShaderModule CreateShaderModule(const std::vector<char>& code);

    class GraphicPipeline
    {
    public:
        GraphicPipeline(std::shared_ptr<PhysicalDevice> device, const std::string& vertexShaderFile,
                        const std::string& fragmentShaderFile, VkExtent2D swapChainExtent);
        ~GraphicPipeline();
    private:
        std::shared_ptr<PhysicalDevice> m_device;

        VkPipelineLayout m_pipelineLayout;
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_fragShaderModule;
    };
}

#endif //VAZTERAN_GRAPHICPIPELINE_HPP
