#include "vzt/Vulkan/Raytracing/AccelerationStructure.hpp"

namespace vzt
{
    VkAccelerationStructureGeometryKHR toVulkan(const GeometryAs& geometryAs)
    {
        VkAccelerationStructureGeometryKHR vkGeometry{};

        vkGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;

        return vkGeometry;
    }

    GeometryAccelerationStructure::GeometryAccelerationStructure(View<Device> device, GeometryAsBuilder builder)
        : m_device(device), m_builder(std::move(builder))
    {
    }

    GeometryAccelerationStructure::GeometryAccelerationStructure(GeometryAccelerationStructure&& other) noexcept {}

    GeometryAccelerationStructure& GeometryAccelerationStructure::operator=(
        GeometryAccelerationStructure&& other) noexcept
    {
        return *this;
    }

    GeometryAccelerationStructure::~GeometryAccelerationStructure() {}
} // namespace vzt
