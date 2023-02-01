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

        VkPhysicalDeviceVulkan12Features features12{};
        features12.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.bufferDeviceAddress = VK_TRUE;
        features.add(features12);

        return features;
    }

    DeviceFeatures DeviceFeatures::rt()
    {
        DeviceFeatures             features;
        VkPhysicalDeviceFeatures2& vkFeatures2 = features.getPhysicalFeatures();
        vkFeatures2.features.samplerAnisotropy = true;

        VkPhysicalDeviceVulkan12Features features12{};
        features12.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.bufferDeviceAddress = VK_TRUE;
        features.add(features12);

        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
        rayQueryFeatures.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
        rayQueryFeatures.rayQuery = VK_TRUE;
        features.add(rayQueryFeatures);

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeatures{};
        raytracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        raytracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
        features.add(raytracingPipelineFeatures);

        VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures{};
        asFeatures.sType                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        asFeatures.accelerationStructure = VK_TRUE;
        features.add(asFeatures);

        return features;
    }

    void DeviceFeatures::add(GenericDeviceFeature feature) { m_features.emplace_back(std::move(feature)); }
    const VkPhysicalDeviceFeatures2& DeviceFeatures::getAllFeatures() const
    {
        if (m_features.empty())
            return m_physicalFeatures;

        m_physicalFeatures.pNext          = &m_features[0];
        GenericDeviceFeature* lastFeature = &m_features[0];
        for (std::size_t i = 1; i < m_features.size(); i++)
        {
            GenericDeviceFeature* next = &m_features[i];
            lastFeature->pNext         = next;
            lastFeature                = next;
        }

        return m_physicalFeatures;
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
