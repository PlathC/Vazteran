#ifndef VZT_DEVICE_HPP
#define VZT_DEVICE_HPP

#include <set>
#include <vector>

#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"

namespace vzt
{
    class Device;
    class Instance;
    class Surface;

    namespace extension
    {
        constexpr const char* VkKHRSwapchain             = "VK_KHR_swapchain";
        constexpr const char* VkKHRGetMemoryRequirements = "VK_KHR_get_memory_requirements2";
        constexpr const char* VkKHRDedicatedAllocation   = "VK_KHR_dedicated_allocation";
    } // namespace extension

    enum class QueueType : uint8_t
    {
        None     = 0,
        Transfer = 1,
        Compute  = 2,
        Graphics = 4
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(QueueType)

    struct DeviceConfiguration
    {
        bool      hasSwapchain  = true;
        bool      hasAnisotropy = true;
        QueueType queueTypes    = QueueType::Graphics | QueueType::Compute;

        // clang-format off
        std::vector<const char*> extensions = {    
            extension::VkKHRSwapchain,             
            extension::VkKHRGetMemoryRequirements, 
            extension::VkKHRDedicatedAllocation
        };
        // clang-format on
    };

    class PhysicalDevice
    {
      public:
        PhysicalDevice(VkPhysicalDevice handle);
        ~PhysicalDevice() = default;

        bool isSuitable(DeviceConfiguration configuration, View<Surface> surface = {}) const;
        bool hasExtensions(const std::vector<const char*>& extensions) const;

        inline VkPhysicalDevice getHandle() const;

      private:
        VkPhysicalDevice m_handle = VK_NULL_HANDLE;
    };

    class Queue
    {
      public:
        Queue(View<Device> device, QueueType type, uint32_t id);
        ~Queue() = default;

        inline QueueType getType() const;

      private:
        View<Device> m_device{};

        VkQueue   m_handle{};
        QueueType m_type;
        uint32_t  m_id;
    };

    class Device
    {
      public:
        Device(View<Instance> instance, PhysicalDevice device, DeviceConfiguration configuration = {});

        Device(const Device&)            = delete;
        Device& operator=(const Device&) = delete;

        Device(Device&&) noexcept;
        Device& operator=(Device&&) noexcept;

        ~Device();

        bool            hasQueue() const;
        inline VkDevice getHandle() const;

      private:
        View<Instance> m_instance;
        PhysicalDevice m_device;
        VkDevice       m_handle    = VK_NULL_HANDLE;
        VmaAllocator   m_allocator = VK_NULL_HANDLE;

        static bool isSameQueue(const Queue& q1, const Queue& q2) { return q1.getType() < q2.getType(); }
        std::set<Queue, decltype(&isSameQueue)> m_queues{&isSameQueue};
    };
} // namespace vzt

#include "vzt/Device.inl"

#endif // VZT_DEVICE_HPP
