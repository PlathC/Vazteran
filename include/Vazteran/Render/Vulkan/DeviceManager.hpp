//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_DEVICEMANAGER_HPP
#define VAZTERAN_DEVICEMANAGER_HPP

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "Vazteran/Render/Vulkan/PhysicalDevice.hpp"

namespace vzt
{
    class Surface;

    using TestSuitableDevice = std::function<bool(VkPhysicalDevice)>;

    class DeviceManager
    {
    public:
        explicit DeviceManager(std::shared_ptr<VkInstance> instance, Surface* surface,
                const std::vector<const char*>& neededDeviceExtensions = {});

        std::shared_ptr<PhysicalDevice> PickedPhysicalDevice();

        ~DeviceManager() = default;

    private:
        PhysicalDevice PickBestSuitableDevice(const std::vector<PhysicalDevice>& devices, Surface* surface);
    private:
        TestSuitableDevice m_suitableDeviceTest;

        uint32_t m_deviceCount = 0;
        std::shared_ptr<VkInstance> m_vkInstance;
        std::shared_ptr<PhysicalDevice> m_physicalDevice;

        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        std::vector<const char*> m_deviceExtensions;

        constexpr static inline std::array<const char*, 1> DefautlDeviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
    };
}


#endif //VAZTERAN_DEVICEMANAGER_HPP
