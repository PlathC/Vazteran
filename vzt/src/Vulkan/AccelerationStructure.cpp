#include "vzt/Vulkan/AccelerationStructure.hpp"

#include "vzt/Vulkan/Command.hpp"
#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    VkAccelerationStructureGeometryKHR toVulkan(const GeometryAsBuilder& geometryAs)
    {
        VkAccelerationStructureGeometryKHR vkGeometry{};
        vkGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        vkGeometry.flags = toVulkan(geometryAs.flags);

        std::visit(
            Overloaded{[&vkGeometry](const AsTriangles& trianglesAs) {
                           vkGeometry.geometryType = toVulkan(GeometryType::Triangles);
                           vkGeometry.geometry.triangles.sType =
                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                           vkGeometry.geometry.triangles.vertexFormat = toVulkan(trianglesAs.vertexFormat);

                           const auto&    vertexBuffer = *trianglesAs.vertexBuffer.data;
                           const uint32_t maxVertex =
                               static_cast<uint32_t>(trianglesAs.vertexBuffer.data->size() / trianglesAs.vertexStride);
                           vkGeometry.geometry.triangles.vertexData.deviceAddress = vertexBuffer.getDeviceAddress();
                           vkGeometry.geometry.triangles.maxVertex                = maxVertex;
                           vkGeometry.geometry.triangles.vertexStride             = trianglesAs.vertexStride;

                           const auto& indexBuffer                                   = *trianglesAs.indexBuffer.data;
                           vkGeometry.geometry.triangles.indexType                   = VK_INDEX_TYPE_UINT32;
                           vkGeometry.geometry.triangles.indexData.deviceAddress     = indexBuffer.getDeviceAddress();
                           vkGeometry.geometry.triangles.transformData.deviceAddress = 0;
                       },
                       [&vkGeometry](const AsInstance& instance) {
                           vkGeometry.geometryType = toVulkan(GeometryType::Instances);
                           vkGeometry.geometry.instances.sType =
                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
                           vkGeometry.geometry.instances.arrayOfPointers    = VK_FALSE;
                           vkGeometry.geometry.instances.data.deviceAddress = instance.deviceAddress;
                       },
                       [&vkGeometry](const AsAabbs& aabbsAs) {
                           vkGeometry.geometryType = toVulkan(GeometryType::AABBs);
                           vkGeometry.geometry.aabbs.sType =
                               VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                           vkGeometry.geometry.aabbs.data.deviceAddress = aabbsAs.aabbs.data->getDeviceAddress();
                           vkGeometry.geometry.aabbs.stride             = aabbsAs.stride;
                       }},
            geometryAs.geometry);

        return vkGeometry;
    }

    AccelerationStructure::AccelerationStructure(View<Device> device, std::vector<GeometryAsBuilder> geometries,
                                                 AccelerationStructureType type)
        : m_device(device), m_geometries(std::move(geometries)), m_type(type)
    {
        std::vector<VkAccelerationStructureGeometryKHR> vkGeometries;
        vkGeometries.reserve(m_geometries.size());
        m_maxPrimitiveCount = 0;
        for (const auto& geometry : m_geometries)
        {
            std::visit(Overloaded{
                           [this](const AsTriangles& trianglesAs) {
                               m_maxPrimitiveCount +=
                                   static_cast<uint32_t>(trianglesAs.indexBuffer.data->size() / (3 * sizeof(uint32_t)));
                           },
                           [this](const AsAabbs& aabbsAs) {
                               m_maxPrimitiveCount += static_cast<uint32_t>(aabbsAs.aabbs.size / aabbsAs.stride);
                           },
                           [this](const AsInstance& instancesAs) { m_maxPrimitiveCount += instancesAs.count; },
                       },
                       geometry.geometry);

            auto vkGeometry = toVulkan(geometry);
            vkGeometries.emplace_back(std::move(vkGeometry));
        }

        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type  = toVulkan(type);
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(vkGeometries.size());
        accelerationStructureBuildGeometryInfo.pGeometries   = vkGeometries.data();

        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkGetAccelerationStructureBuildSizesKHR(
            m_device->getHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &accelerationStructureBuildGeometryInfo, &m_maxPrimitiveCount, &accelerationStructureBuildSizesInfo);

        m_size              = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        m_buffer            = Buffer( //
            m_device, m_size, BufferUsage::AccelerationStructureStorage | BufferUsage::ShaderDeviceAddress);
        m_scratchBufferSize = accelerationStructureBuildSizesInfo.buildScratchSize;

        // Acceleration structure
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
        accelerationStructureCreate_info.sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreate_info.buffer = m_buffer.getHandle();
        accelerationStructureCreate_info.size   = m_size;
        accelerationStructureCreate_info.type   = toVulkan(type);

        vkCheck(table.vkCreateAccelerationStructureKHR( //
                    m_device->getHandle(), &accelerationStructureCreate_info, nullptr, &m_handle),
                "Failed to create acceleration structure.");

        // AS device address
        VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
        accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        accelerationDeviceAddressInfo.accelerationStructure = m_handle;
        m_deviceAddress                                     = table.vkGetAccelerationStructureDeviceAddressKHR( //
            m_device->getHandle(), &accelerationDeviceAddressInfo);
    }

    AccelerationStructure::AccelerationStructure(View<Device> device, GeometryAsBuilder geometry,
                                                 AccelerationStructureType type)
        : AccelerationStructure(device, std::vector{geometry}, type)
    {
    }

    AccelerationStructure::AccelerationStructure(AccelerationStructure&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_device, other.m_device);
        std::swap(m_geometries, other.m_geometries);
        std::swap(m_type, other.m_type);
        std::swap(m_maxPrimitiveCount, other.m_maxPrimitiveCount);
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_deviceAddress, other.m_deviceAddress);
        std::swap(m_size, other.m_size);
        std::swap(m_scratchBufferSize, other.m_scratchBufferSize);
    }

    AccelerationStructure& AccelerationStructure::operator=(AccelerationStructure&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_device, other.m_device);
        std::swap(m_geometries, other.m_geometries);
        std::swap(m_type, other.m_type);
        std::swap(m_maxPrimitiveCount, other.m_maxPrimitiveCount);
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_deviceAddress, other.m_deviceAddress);
        std::swap(m_size, other.m_size);
        std::swap(m_scratchBufferSize, other.m_scratchBufferSize);

        return *this;
    }

    AccelerationStructure::~AccelerationStructure()
    {
        if (m_handle == VK_NULL_HANDLE)
            return;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkDestroyAccelerationStructureKHR(m_device->getHandle(), m_handle, nullptr);
    }
} // namespace vzt
