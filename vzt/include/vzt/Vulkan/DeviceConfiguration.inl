#include "vzt/Vulkan/DeviceConfiguration.hpp"

namespace vzt
{
    inline const std::vector<GenericDeviceFeature>& DeviceFeatures::getFeatures() const { return m_features; }
    inline const VkPhysicalDeviceFeatures2& DeviceFeatures::getPhysicalFeatures() const { return m_physicalFeatures; }
    inline VkPhysicalDeviceFeatures2&       DeviceFeatures::getPhysicalFeatures() { return m_physicalFeatures; }

    inline void DeviceBuilder::set(DeviceFeatures features) { m_features = std::move(features); }
    inline void DeviceBuilder::add(QueueType queueType) { m_queueTypes |= queueType; }
    inline void DeviceBuilder::add(dext::Extension extension) { m_extensions.emplace_back(std::move(extension)); }

    inline const DeviceFeatures&               DeviceBuilder::getDeviceFeatures() const { return m_features; }
    inline QueueType                           DeviceBuilder::getQueueTypes() const { return m_queueTypes; }
    inline const std::vector<dext::Extension>& DeviceBuilder::getExtensions() const { return m_extensions; }
} // namespace vzt
