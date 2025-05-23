#ifndef VZT_VULKAN_DEVICE_HPP
#define VZT_VULKAN_DEVICE_HPP

#include <functional>
#include <set>
#include <vector>

#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Vulkan/DeviceConfiguration.hpp"
#include "vzt/Vulkan/Format.hpp"

namespace vzt
{
    class Device;
    class Instance;
    class Surface;

    class PhysicalDevice
    {
      public:
        PhysicalDevice() = default;
        PhysicalDevice(VkPhysicalDevice handle);

        bool                                 isSuitable(DeviceBuilder configuration, View<Surface> surface = {}) const;
        bool                                 hasExtensions(const std::vector<const char*>& extensions) const;
        std::vector<VkQueueFamilyProperties> getQueueFamiliesProperties() const;
        bool                                 canQueueFamilyPresent(uint32_t id, View<Surface> surface) const;
        Format                               getDepthFormat() const;

        std::size_t getUniformAlignment(std::size_t alignment) const;
        template <class Type>
        std::size_t getUniformAlignment() const;

        inline VkPhysicalDeviceProperties getProperties() const;
        inline VkPhysicalDevice           getHandle() const;

      private:
        VkPhysicalDevice           m_handle = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_properties;
    };

    class Queue;
    class Device
    {
      public:
        Device() = default;
        Device(View<Instance> instance, PhysicalDevice device, DeviceBuilder configuration = {},
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

        inline VkDevice               getHandle() const;
        inline const VolkDeviceTable& getFunctionTable() const;
        inline VmaAllocator           getAllocator() const;
        inline PhysicalDevice         getHardware() const;

      private:
        View<Instance>  m_instance;
        PhysicalDevice  m_device;
        VolkDeviceTable m_table;

        VkDevice      m_handle    = VK_NULL_HANDLE;
        VmaAllocator  m_allocator = VK_NULL_HANDLE;
        DeviceBuilder m_configuration;

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
        void submit(const CommandBuffer& commandBuffer, const SwapchainSubmission& submission) const;
        void submit(const CommandBuffer& commandBuffer) const;

        inline View<Device> getDevice() const;
        inline VkQueue      getHandle() const;
        inline QueueType    getType() const;
        inline uint32_t     getId() const;
        inline bool         canPresent() const;

      private:
        View<Device> m_device{};

        VkQueue   m_handle{};
        QueueType m_type;
        uint32_t  m_id;
        bool      m_canPresent = false;
    };
} // namespace vzt

#include "vzt/Vulkan/Device.inl"

#endif // VZT_VULKAN_DEVICE_HPP
