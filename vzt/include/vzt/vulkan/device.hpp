#ifndef VZT_VULKAN_DEVICE_HPP
#define VZT_VULKAN_DEVICE_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "vzt/core/type.hpp"
#include "vzt/vulkan/type.hpp"

#ifndef VK_DEFINE_NON_DISPATCHABLE_HANDLE
#if (VK_USE_64_BIT_PTR_DEFINES == 1)
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201103L))
#define VK_NULL_HANDLE nullptr
#else
#define VK_NULL_HANDLE ((void*)0)
#endif
#else
#define VK_NULL_HANDLE 0ULL
#endif
#endif
#ifndef VK_NULL_HANDLE
#define VK_NULL_HANDLE 0
#endif

namespace vzt
{
    class Device;
    class Instance;
    class Surface;

    namespace dext
    {
        using Extension = const char*;

        constexpr Extension Swapchain               = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        constexpr Extension GetMemoryRequirements2  = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
        constexpr Extension DedicatedAllocation     = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
        constexpr Extension AccelerationStructure   = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
        constexpr Extension RaytracingPipeline      = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
        constexpr Extension BufferDeviceAddress     = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
        constexpr Extension DeferredHostOperations  = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
        constexpr Extension DescriptorIndexing      = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
        constexpr Extension Spirv14                 = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
        constexpr Extension ShaderFloatControls     = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
        constexpr Extension GraphicsPipelineLibrary = VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME;
        constexpr Extension PortabilitySubset       = "VK_KHR_portability_subset";
        constexpr Extension NonSemanticInfo         = VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME;
        constexpr Extension DynamicRendering        = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
        constexpr Extension MemoryBudget            = VK_EXT_MEMORY_BUDGET_EXTENSION_NAME;
    } // namespace dext

    struct DeviceFeatures
    {
        static DeviceFeatures standard();
        static DeviceFeatures rt();

        VkPhysicalDeviceFeatures         core{};
        VkPhysicalDeviceVulkan11Features v11{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
        VkPhysicalDeviceVulkan12Features v12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};

        // Returns the structure of the requested type, adding it to the chain if needed
        template <class Type>
        Type& get();

        static constexpr uint32_t NoField = 0xffffffff;

        // Returns the first feature enabled but not in supported, or an empty string if there is none
        std::string findMissing(const DeviceFeatures& supported) const;

        // Links every structure and returns the head of the chain, to be given to Vulkan
        VkPhysicalDeviceFeatures2& chain();

      private:
        VkPhysicalDeviceFeatures2           m_head{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        std::vector<std::vector<std::byte>> m_others; // Zero-initialized storage of each structure added by get()
    };

    struct Capability
    {
        std::string                          name;
        std::vector<dext::Extension>         extensions = {};
        std::function<void(DeviceFeatures&)> enable     = {};
    };

    namespace cap
    {
        inline const Capability MemoryBudget        = {.name = "MemoryBudget", .extensions = {dext::MemoryBudget}};
        inline const Capability BufferDeviceAddress = {
            .name   = "BufferDeviceAddress",
            .enable = [](DeviceFeatures& features) { features.v12.bufferDeviceAddress = VK_TRUE; },
        };
    } // namespace cap

    class DeviceBuilder
    {
      public:
        static DeviceBuilder offline();
        static DeviceBuilder standard();
        static DeviceBuilder rt();

        DeviceBuilder()  = default;
        ~DeviceBuilder() = default;

        inline void set(DeviceFeatures features);
        inline void add(QueueType queueType);
        inline void add(dext::Extension extension);

        inline DeviceBuilder& require(Capability capability); // A device without it is not suitable
        inline DeviceBuilder& request(Capability capability); // Enabled only if the device supports it

        inline const DeviceFeatures&               getDeviceFeatures() const;
        inline DeviceFeatures&                     getDeviceFeatures();
        inline QueueType                           getQueueTypes() const;
        inline const std::vector<dext::Extension>& getExtensions() const;
        inline const std::vector<Capability>&      getRequiredCapabilities() const;
        inline const std::vector<Capability>&      getRequestedCapabilities() const;

      private:
        DeviceFeatures m_features;
        QueueType      m_queueTypes = QueueType::None;

        std::vector<dext::Extension> m_extensions;
        std::vector<Capability>      m_required;
        std::vector<Capability>      m_requested;
    };

    class PhysicalDevice
    {
      public:
        PhysicalDevice() = default;
        PhysicalDevice(VkPhysicalDevice handle);

        bool isSuitable(DeviceBuilder configuration, View<Surface> surface = {}) const;
        // Returns the support of every structure present in features
        DeviceFeatures                       getFeatures(DeviceFeatures features = {}) const;
        bool                                 hasExtensions(const std::vector<const char*>& extensions) const;
        std::set<std::string>                getMissingExtensions(const std::vector<const char*>& extensions) const;
        bool                                 supports(const Capability& capability) const;
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

        // Returns true if the capability has been enabled on this device
        bool has(const Capability& capability) const;

        inline VkDevice               getHandle() const;
        inline const VolkDeviceTable& getFunctionTable() const;
        inline VmaAllocator           getAllocator() const;
        inline PhysicalDevice         getHardware() const;

      private:
        View<Instance>  m_instance;
        PhysicalDevice  m_device;
        VolkDeviceTable m_table;

        VkDevice                 m_handle    = VK_NULL_HANDLE;
        VmaAllocator             m_allocator = VK_NULL_HANDLE;
        DeviceBuilder            m_configuration;
        std::vector<std::string> m_capabilities;

        static inline bool                      isSameQueue(const Queue& q1, const Queue& q2);
        std::set<Queue, decltype(&isSameQueue)> m_queues{&isSameQueue};
    };

    template <class Handle>
    class DeviceObject
    {
      public:
        DeviceObject() = default;
        DeviceObject(View<Device> device, Handle handle = nullptr);

        DeviceObject(const DeviceObject&)            = delete;
        DeviceObject& operator=(const DeviceObject&) = delete;

        DeviceObject(DeviceObject&& other) noexcept;
        DeviceObject& operator=(DeviceObject&& other) noexcept;

        virtual ~DeviceObject() = default;

        View<Device>  getDevice() const;
        const Handle& getHandle() const;

      protected:
        View<Device> m_device;
        Handle       m_handle = VK_NULL_HANDLE;
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

#include "vzt/vulkan/device.inl"

#endif // VZT_VULKAN_DEVICE_HPP
