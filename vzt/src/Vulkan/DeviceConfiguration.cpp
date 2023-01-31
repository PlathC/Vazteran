#include "vzt/Vulkan/DeviceConfiguration.hpp"

namespace vzt
{
    bool GenericDeviceFeature::match(const GenericDeviceFeature& requested, const GenericDeviceFeature& supported)
    {
        for (std::size_t i = 0; i < MaximumFieldCount; i++)
        {
            if (requested.fields[i] && !supported.fields[i])
                return false;
        }

        return true;
    }

    DeviceFeatures DeviceFeatures::standard()
    {
        DeviceFeatures             features;
        VkPhysicalDeviceFeatures2& vkFeatures2 = features.getPhysicalFeatures();
        vkFeatures2.features.samplerAnisotropy = true;

        return features;
    }

    DeviceFeatures DeviceFeatures::rt()
    {
        DeviceFeatures             features;
        VkPhysicalDeviceFeatures2& vkFeatures2 = features.getPhysicalFeatures();
        vkFeatures2.features.samplerAnisotropy = true;

        return features;
    }

    inline void DeviceFeatures::add(GenericDeviceFeature feature)
    {
        m_features.emplace_back(std::move(feature));
        if (m_features.size() == 1)
            m_physicalFeatures.pNext = &m_features.back();
        else
            m_features[m_features.size() - 1].pNext = &m_features.back();
    }

    DeviceBuilder DeviceBuilder::standard()
    {
        DeviceBuilder builder{};
        builder.m_features   = DeviceFeatures::standard();
        builder.m_extensions = {dext::Swapchain, dext::GetMemoryRequirements2, dext::DedicatedAllocation};
        builder.m_queueTypes = QueueType::Graphics | QueueType::Compute;

        return builder;
    }

    DeviceBuilder DeviceBuilder::rt()
    {
        DeviceBuilder builder{};
        builder.m_features   = DeviceFeatures::rt();
        builder.m_extensions = {dext::Swapchain,
                                dext::GetMemoryRequirements2,
                                dext::DedicatedAllocation,
                                dext::AccelerationStructure,
                                dext::RaytracingPipeline,
                                dext::BufferDeviceAddress,
                                dext::DeferredHostOperations,
                                dext::DescriptorIndexing,
                                dext::Spirv14,
                                dext::ShaderFloatControls};
        builder.m_queueTypes = QueueType::Graphics | QueueType::Compute;

        return builder;
    }
} // namespace vzt
