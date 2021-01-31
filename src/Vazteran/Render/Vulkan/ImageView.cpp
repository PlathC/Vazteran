//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#include "Vazteran/Render/Vulkan/ImageView.hpp"

namespace vzt
{
    ImageView::ImageView(std::shared_ptr<PhysicalDevice> physicalDevice, VkImage swapChainImage, VkFormat swapChainImageFormat):
        m_physicalDevice(std::move(physicalDevice))
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImage;

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_physicalDevice->DeviceHandler(), &createInfo, nullptr, &m_imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }

    ImageView::~ImageView()
    {
        vkDestroyImageView(m_physicalDevice->DeviceHandler(), m_imageView, nullptr);
    }
}