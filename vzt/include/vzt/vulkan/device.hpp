#ifndef VZT_VULKAN_DEVICE_HPP
#define VZT_VULKAN_DEVICE_HPP

#include <functional>
#include <set>
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
    } // namespace dext

    // Based on https://github.com/charles-lunarg/vk-bootstrap/blob/master/src/VkBootstrap.h#L161
    // Copyright � 2020 Charles Giessen (charles@lunarg.com)
    // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
    // documentation files (the �Software�), to deal in the Software without restriction, including without limitation
    // the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
    // to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above
    // copyright notice and this permission notice shall be included in all copies or substantial portions of the
    // Software. THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
    // LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
    // SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    // OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    // DEALINGS IN THE SOFTWARE.
    struct GenericDeviceFeature
    {
        static const uint32_t MaximumFieldCount = 256;

        template <class GenericDeviceFeatureT>
        GenericDeviceFeature(const GenericDeviceFeatureT& features)
        {
            std::memset(fields, 0xff, sizeof(VkBool32) * MaximumFieldCount);
            std::memcpy(this, &features, sizeof(GenericDeviceFeatureT));
        }

        static bool match(const GenericDeviceFeature& requested, const GenericDeviceFeature& supported);

        VkStructureType sType = static_cast<VkStructureType>(0);
        void*           pNext = nullptr;
        VkBool32        fields[MaximumFieldCount];
    };

    class DeviceFeatures
    {
      public:
        static DeviceFeatures standard();
        static DeviceFeatures rt();

        void                                            add(GenericDeviceFeature feature);
        inline const std::vector<GenericDeviceFeature>& getFeatures() const;
        inline const VkPhysicalDeviceFeatures2&         getPhysicalFeatures() const;
        inline VkPhysicalDeviceFeatures2&               getPhysicalFeatures();
        const VkPhysicalDeviceFeatures2&                getAllFeatures() const;

      private:
        // Mutable to allow dynamic chain creation
        mutable VkPhysicalDeviceFeatures2 m_physicalFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr, {}};
        mutable std::vector<GenericDeviceFeature> m_features;
    };

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

        inline const DeviceFeatures&               getDeviceFeatures() const;
        inline DeviceFeatures&                     getDeviceFeatures();
        inline QueueType                           getQueueTypes() const;
        inline const std::vector<dext::Extension>& getExtensions() const;

      private:
        DeviceFeatures m_features;
        QueueType      m_queueTypes;

        std::vector<dext::Extension> m_extensions;
    };

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
