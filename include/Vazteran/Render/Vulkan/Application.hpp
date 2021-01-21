//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_APPLICATION_HPP
#define VAZTERAN_APPLICATION_HPP

#include <array>
#include <functional>
#include <vector>

#include "Vazteran/Render/Vulkan/ErrorHandler.hpp"
#include "Vazteran/Render/Vulkan/DeviceManager.hpp"
#include "Vazteran/Render/Vulkan/Surface.hpp"

namespace vzt
{

    class Application
    {
    public:
        Application(std::vector<const char*> extensions, SurfaceInitializer surfaceInitializer);

        ~Application();
    private:
        bool CheckValidationLayerSupport();

        static constexpr std::array<const char*, 1> ValidationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };

    private:
        std::unique_ptr<Surface> m_surface;
        std::unique_ptr<DebugMessenger> m_debugMessenger;
        std::unique_ptr<DeviceManager> m_deviceManager;
        std::shared_ptr<VkInstance> m_vkInstance;

#ifdef NDEBUG
        static constexpr bool enableValidationLayers = false;
#else
        static constexpr bool enableValidationLayers = true;
#endif
    };
}


#endif //VAZTERAN_APPLICATION_HPP
