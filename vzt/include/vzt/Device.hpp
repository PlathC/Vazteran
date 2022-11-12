#ifndef VZT_DEVICE_HPP
#define VZT_DEVICE_HPP

#include <functional>
#include <optional>
#include <set>
#include <vector>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

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
        Transfer = VK_QUEUE_TRANSFER_BIT,
        Compute  = VK_QUEUE_COMPUTE_BIT,
        Graphics = VK_QUEUE_GRAPHICS_BIT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(QueueType, VkQueueFlagBits)
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

        bool isSuitable(DeviceConfiguration configuration, View<Surface> surface = {}) const;
        bool hasExtensions(const std::vector<const char*>& extensions) const;
        std::vector<VkQueueFamilyProperties> getQueueFamiliesProperties() const;
        bool                                 canQueueFamilyPresent(uint32_t id, View<Surface> surface) const;

        inline VkPhysicalDevice getHandle() const;

      private:
        VkPhysicalDevice m_handle = VK_NULL_HANDLE;
    };

    class Queue;
    class Device
    {
      public:
        Device(View<Instance> instance, PhysicalDevice device, DeviceConfiguration configuration = {},
               View<Surface> surface = {});

        Device(const Device&)            = delete;
        Device& operator=(const Device&) = delete;

        Device(Device&&) noexcept;
        Device& operator=(Device&&) noexcept;

        ~Device();

        void wait() const;

        std::vector<View<Queue>> getQueues() const;
        View<Queue>              getQueue(QueueType type) const;
        View<Queue>              getPresentQueue() const;

        inline VkDevice       getHandle() const;
        inline VmaAllocator   getAllocator() const;
        inline PhysicalDevice getHardware() const;

      private:
        View<Instance> m_instance;
        PhysicalDevice m_device;

        VkDevice            m_handle    = VK_NULL_HANDLE;
        VmaAllocator        m_allocator = VK_NULL_HANDLE;
        DeviceConfiguration m_configuration;

        static inline bool                      isSameQueue(const Queue& q1, const Queue& q2);
        std::set<Queue, decltype(&isSameQueue)> m_queues{&isSameQueue};
    };

    class CommandBuffer;
    struct SwapchainSubmission;
    class Queue
    {
      public:
        Queue(View<Device> device, QueueType type, uint32_t id, bool canPresent = false);
        ~Queue() = default;

        using SingleTimeCommandFunction = std::function<void(CommandBuffer&)>;
        void oneShot(const SingleTimeCommandFunction& function) const;
        void submit(CommandBuffer& commandBuffer, const SwapchainSubmission& submission) const;
        void submit(CommandBuffer& commandBuffer) const;

        inline VkQueue   getHandle() const;
        inline QueueType getType() const;
        inline uint32_t  getId() const;
        inline bool      canPresent() const;

      private:
        View<Device> m_device{};

        VkQueue   m_handle{};
        QueueType m_type;
        uint32_t  m_id;
        bool      m_canPresent = false;
    };
} // namespace vzt

#include "vzt/Device.inl"

#endif // VZT_DEVICE_HPP
