#include "vzt/Vulkan/Raytracing/AccelerationStructure.hpp"

#include "vzt/Vulkan/Command.hpp"
#include "vzt/Vulkan/Device.hpp"

namespace vzt
{
    VkAccelerationStructureGeometryKHR toVulkan(const GeometryAsBuilder& geometryAs)
    {
        VkAccelerationStructureGeometryKHR vkGeometry{};
        vkGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        vkGeometry.flags = toVulkan(geometryAs.flags);

        // clang-format off
        std::visit(Overloaded{
            [&vkGeometry](const AsTriangles& trianglesAs) {
                vkGeometry.geometryType                    = toVulkan(GeometryType::Triangles);
                vkGeometry.geometry.triangles.sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                vkGeometry.geometry.triangles.vertexFormat = toVulkan(trianglesAs.vertexFormat);

                const auto& vertexBuffer = *trianglesAs.vertexBuffer.data;
                const uint32_t maxVertex = static_cast<uint32_t>(trianglesAs.vertexBuffer.data->size() / trianglesAs.vertexStride);
                vkGeometry.geometry.triangles.vertexData.deviceAddress = vertexBuffer.getDeviceAddress();
                vkGeometry.geometry.triangles.maxVertex                = maxVertex;
                vkGeometry.geometry.triangles.vertexStride             = trianglesAs.vertexStride;

                const auto& indexBuffer = *trianglesAs.indexBuffer.data;
                vkGeometry.geometry.triangles.indexType                   = VK_INDEX_TYPE_UINT32;
                vkGeometry.geometry.triangles.indexData.deviceAddress     = indexBuffer.getDeviceAddress();
                vkGeometry.geometry.triangles.transformData.deviceAddress = 0;

            },
            [&vkGeometry](const AsAabbs& aabbsAs) {
                vkGeometry.geometry.aabbs.sType              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                vkGeometry.geometryType                      = toVulkan(GeometryType::AABBs);
                vkGeometry.geometry.aabbs.data.deviceAddress = aabbsAs.aabbs.data->getDeviceAddress();
                vkGeometry.geometry.aabbs.stride             = aabbsAs.stride;

            }},
            geometryAs.geometry
        );
        // clang-format on

        return vkGeometry;
    }

    BottomGeometryAs::BottomGeometryAs(View<Device> device, std::vector<GeometryAsBuilder> geometries,
                                       BuildAccelerationStructureFlag flags)
        : m_device(device), m_geometries(std::move(geometries))
    {
        std::vector<VkAccelerationStructureGeometryKHR> vkGeometries;
        vkGeometries.reserve(m_geometries.size());
        uint32_t maxPrimitiveCount = 0;
        for (const auto& geometry : m_geometries)
        {
            std::visit(Overloaded{[&maxPrimitiveCount](const AsTriangles& trianglesAs) {
                                      maxPrimitiveCount +=
                                          static_cast<uint32_t>(trianglesAs.indexBuffer.size / sizeof(uint32_t));
                                  },
                                  [&maxPrimitiveCount](const AsAabbs& aabbsAs) {
                                      maxPrimitiveCount += static_cast<uint32_t>(aabbsAs.aabbs.size / aabbsAs.stride);
                                  }},
                       geometry.geometry);

            auto vkGeometry = toVulkan(geometry);
            vkGeometries.emplace_back(std::move(vkGeometry));
        }

        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type  = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(vkGeometries.size());
        accelerationStructureBuildGeometryInfo.pGeometries   = vkGeometries.data();

        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        const VolkDeviceTable& table = m_device->getFunctionTable();
        table.vkGetAccelerationStructureBuildSizesKHR(
            m_device->getHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &accelerationStructureBuildGeometryInfo, &maxPrimitiveCount, &accelerationStructureBuildSizesInfo);

        m_size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        m_buffer =
            Buffer(m_device, m_size, BufferUsage::AccelerationStructureStorage | BufferUsage::ShaderDeviceAddress);

        const uint64_t scratchBufferSize = accelerationStructureBuildSizesInfo.buildScratchSize;
        auto           scratchBuffer =
            Buffer(m_device, scratchBufferSize, BufferUsage::StorageBuffer | BufferUsage::ShaderDeviceAddress);

        // Acceleration structure
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
        accelerationStructureCreate_info.sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreate_info.buffer = m_buffer.getHandle();
        accelerationStructureCreate_info.size   = m_size;
        accelerationStructureCreate_info.type   = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

        vkCheck(table.vkCreateAccelerationStructureKHR(m_device->getHandle(), &accelerationStructureCreate_info,
                                                       nullptr, &m_handle),
                "Failed to create acceleration structure.");

        // AS device address
        VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
        accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        accelerationDeviceAddressInfo.accelerationStructure = m_handle;

        m_deviceAddress =
            table.vkGetAccelerationStructureDeviceAddressKHR(m_device->getHandle(), &accelerationDeviceAddressInfo);

        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationBuildGeometryInfo.type  = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationBuildGeometryInfo.mode  = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationBuildGeometryInfo.dstAccelerationStructure  = m_handle;
        accelerationBuildGeometryInfo.geometryCount             = static_cast<uint32_t>(vkGeometries.size());
        accelerationBuildGeometryInfo.pGeometries               = vkGeometries.data();
        accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getDeviceAddress();

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount  = maxPrimitiveCount;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex     = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;

        std::vector accelerationBuildStructureRangeInfos = {&accelerationStructureBuildRangeInfo};

        // "vkCmdBuildAccelerationStructuresKHR Supported Queue Types: Compute"
        const View<Queue> queue = m_device->getQueue(QueueType::Compute);
        queue->oneShot([&](CommandBuffer& commands) {
            BottomGeometryAsBuilder builder{flags, this, scratchBuffer, m_geometries};
            commands.buildAs(builder);
        });
    }

    BottomGeometryAs::BottomGeometryAs(View<Device> device, GeometryAsBuilder geometry,
                                       BuildAccelerationStructureFlag flags)
        : BottomGeometryAs(device, std::vector{std::move(geometry)}, flags)
    {
    }

    BottomGeometryAs::BottomGeometryAs(BottomGeometryAs&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_device, other.m_device);
        std::swap(m_geometries, other.m_geometries);
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_deviceAddress, other.m_deviceAddress);
        std::swap(m_size, other.m_size);
    }

    BottomGeometryAs& BottomGeometryAs::operator=(BottomGeometryAs&& other) noexcept
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_device, other.m_device);
        std::swap(m_geometries, other.m_geometries);
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_deviceAddress, other.m_deviceAddress);
        std::swap(m_size, other.m_size);

        return *this;
    }

    BottomGeometryAs::~BottomGeometryAs()
    {
        if (m_handle != VK_NULL_HANDLE)
        {
            const VolkDeviceTable& table = m_device->getFunctionTable();
            table.vkDestroyAccelerationStructureKHR(m_device->getHandle(), m_handle, nullptr);
        }
    }
} // namespace vzt
