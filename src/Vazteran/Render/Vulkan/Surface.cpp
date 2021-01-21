//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#include "Vazteran/Render/Vulkan/Surface.hpp"

namespace vzt
{
    Surface::Surface(std::shared_ptr<VkInstance> instance, SurfaceInitializer surfaceInitializer):
        m_instance(std::move(instance))
    {
        surfaceInitializer(*m_instance, m_surface);
    }

    VkBool32 Surface::CheckSupport(VkPhysicalDevice device, uint32_t queueFamilyIndex)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, m_surface, &presentSupport);
        return presentSupport;
    }

    Surface::~Surface()
    {
        vkDestroySurfaceKHR(*m_instance, m_surface, nullptr);
    }
}