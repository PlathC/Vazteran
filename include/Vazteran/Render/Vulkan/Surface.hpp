//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_SURFACE_HPP
#define VAZTERAN_SURFACE_HPP

#include <functional>
#include <memory>

#include <vulkan/vulkan.h>

namespace vzt
{
    using SurfaceInitializer = std::function<void(VkInstance, VkSurfaceKHR&)>;

    class Surface
    {
    public:
        Surface(std::shared_ptr<VkInstance> instance, SurfaceInitializer surfaceInitializer);

        VkBool32 CheckSupport(VkPhysicalDevice device, uint32_t queueFamilyIndex);
        VkSurfaceKHR& SurfaceHandler();

        ~Surface();
    private:
        std::shared_ptr<VkInstance> m_instance;
        VkSurfaceKHR m_surface;
    };
}

#endif //VAZTERAN_SURFACE_HPP
