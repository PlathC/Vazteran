#ifndef VZT_VULKAN_RAYTRACING_ACCELERATIONSTRUCTURE_HPP
#define VZT_VULKAN_RAYTRACING_ACCELERATIONSTRUCTURE_HPP

#include <variant>

#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"
#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/Format.hpp"

namespace vzt
{
    class Device;

    enum class GeometryType : uint8_t
    {
        Triangles = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        AABBs     = VK_GEOMETRY_TYPE_AABBS_KHR,
        Instances = VK_GEOMETRY_TYPE_INSTANCES_KHR
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(GeometryType, VkGeometryTypeKHR)

    enum class GeometryFlag : uint8_t
    {
        Opaque                      = VK_GEOMETRY_OPAQUE_BIT_KHR,
        NoDuplicateAnyHitInvocation = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(GeometryFlag, VkGeometryFlagsKHR)
    VZT_DEFINE_BITWISE_FUNCTIONS(GeometryFlag)

    enum class BuildAccelerationStructure
    {
        AllowUpdate     = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
        AllowCompaction = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR,
        PreferFastTrace = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        PreferFastBuild = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR,
        LowMemory       = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(BuildAccelerationStructure, VkBuildAccelerationStructureFlagsKHR)
    VZT_DEFINE_BITWISE_FUNCTIONS(BuildAccelerationStructure)

    struct TrianglesAs
    {
        Format      vertexFormat;
        BufferCSpan vertexBuffer;
        std::size_t vertexStride;
        std::size_t maxVertex;
        BufferCSpan indexBuffer;

        Optional<BufferCSpan> transformBuffer;
    };

    using AabbPositions = VkAabbPositionsKHR;
    struct AabbsAs
    {
        BufferCSpan aabbs; // Must contain AabbPositions
        std::size_t stride = 0;
    };

    using GeometryAs = std::variant<TrianglesAs, AabbsAs>;

    struct GeometryAsBuilder
    {
        GeometryAs   geometry;
        GeometryFlag flags = GeometryFlag::Opaque;
    };

    class GeometryAccelerationStructure
    {
      public:
        GeometryAccelerationStructure(View<Device> device, GeometryAsBuilder builder);

        GeometryAccelerationStructure(const GeometryAccelerationStructure&)            = delete;
        GeometryAccelerationStructure& operator=(const GeometryAccelerationStructure&) = delete;

        GeometryAccelerationStructure(GeometryAccelerationStructure&&) noexcept;
        GeometryAccelerationStructure& operator=(GeometryAccelerationStructure&&) noexcept;

        ~GeometryAccelerationStructure();

      private:
        VkAccelerationStructureKHR m_handle;
        View<Device>               m_device;
        GeometryAsBuilder          m_builder;
    };

} // namespace vzt

#endif // VZT_VULKAN_RAYTRACING_ACCELERATIONSTRUCTURE_HPP
