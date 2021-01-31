//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_IMAGEVIEW_HPP
#define VAZTERAN_IMAGEVIEW_HPP

#include <memory>

#include "Vazteran/Render/Vulkan/PhysicalDevice.hpp"

namespace vzt
{
    class ImageView
    {
    public:
        ImageView(std::shared_ptr<PhysicalDevice> physicalDevice, VkImage swapChainImage, VkFormat swapChainImageFormat);

        ~ImageView();
    private:
        std::shared_ptr<PhysicalDevice> m_physicalDevice;
        VkImageView m_imageView;
    };
}


#endif //VAZTERAN_IMAGEVIEW_HPP
