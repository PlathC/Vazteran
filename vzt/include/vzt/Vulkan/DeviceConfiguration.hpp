#ifndef VZT_VULKAN_DEVICECONFIGURATION_HPP
#define VZT_VULKAN_DEVICECONFIGURATION_HPP

#include <cstring>
#include <vector>

#include "vzt/Core/Meta.hpp"
#include "vzt/Vulkan/Core.hpp"

namespace vzt
{
    namespace dext
    {
        using Extension = const char*;

        constexpr const char* Swapchain              = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        constexpr const char* GetMemoryRequirements2 = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
        constexpr const char* DedicatedAllocation    = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
        constexpr const char* AccelerationStructure  = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
        constexpr const char* RaytracingPipeline     = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
        constexpr const char* BufferDeviceAddress    = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
        constexpr const char* DeferredHostOperations = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
        constexpr const char* DescriptorIndexing     = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
        constexpr const char* Spirv14                = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
        constexpr const char* ShaderFloatControls    = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
    } // namespace dext

    // Based on https://github.com/charles-lunarg/vk-bootstrap/blob/master/src/VkBootstrap.h#L161
    // Copyright © 2020 Charles Giessen (charles@lunarg.com)
    // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
    // documentation files (the “Software”), to deal in the Software without restriction, including without limitation
    // the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
    // to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above
    // copyright notice and this permission notice shall be included in all copies or substantial portions of the
    // Software. THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
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

        inline void                                     add(GenericDeviceFeature feature);
        inline const std::vector<GenericDeviceFeature>& getFeatures() const;
        inline const VkPhysicalDeviceFeatures2&         getPhysicalFeatures() const;
        inline VkPhysicalDeviceFeatures2&               getPhysicalFeatures();

      private:
        VkPhysicalDeviceFeatures2         m_physicalFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        std::vector<GenericDeviceFeature> m_features;
    };

    enum class QueueType : uint8_t
    {
        None     = 0,
        Graphics = VK_QUEUE_GRAPHICS_BIT,
        Compute  = VK_QUEUE_COMPUTE_BIT,
        Transfer = VK_QUEUE_TRANSFER_BIT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(QueueType, VkQueueFlagBits)
    VZT_DEFINE_BITWISE_FUNCTIONS(QueueType)

    class DeviceBuilder
    {
      public:
        static DeviceBuilder standard();
        static DeviceBuilder rt();

        DeviceBuilder()  = default;
        ~DeviceBuilder() = default;

        inline void set(DeviceFeatures features);
        inline void add(QueueType queueType);
        inline void add(dext::Extension extension);

        inline const DeviceFeatures&               getDeviceFeatures() const;
        inline QueueType                           getQueueTypes() const;
        inline const std::vector<dext::Extension>& getExtensions() const;

      private:
        DeviceFeatures m_features;
        QueueType      m_queueTypes;

        std::vector<dext::Extension> m_extensions;
    };
} // namespace vzt

#include "vzt/Vulkan/DeviceConfiguration.inl"

#endif // VZT_VULKAN_DEVICECONFIGURATION_HPP
