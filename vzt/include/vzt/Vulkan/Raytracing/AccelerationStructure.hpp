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

    enum class BuildAccelerationStructureFlag
    {
        AllowUpdate     = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
        AllowCompaction = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR,
        PreferFastTrace = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        PreferFastBuild = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR,
        LowMemory       = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(BuildAccelerationStructureFlag, VkBuildAccelerationStructureFlagsKHR)
    VZT_DEFINE_BITWISE_FUNCTIONS(BuildAccelerationStructureFlag)

    struct AsTriangles
    {
        Format      vertexFormat;
        BufferCSpan vertexBuffer;
        std::size_t vertexStride;
        std::size_t maxVertex;
        BufferCSpan indexBuffer;

        Optional<BufferCSpan> transformBuffer;
    };

    using AabbPositions = VkAabbPositionsKHR;
    struct AsAabbs
    {
        BufferCSpan aabbs; // Must contain AabbPositions
        std::size_t stride = 0;
    };

    using AsGeometry = std::variant<AsTriangles, AsAabbs>;

    struct GeometryAsBuilder
    {
        AsGeometry   geometry;
        GeometryFlag flags = GeometryFlag::Opaque;
    };
    VkAccelerationStructureGeometryKHR toVulkan(const GeometryAsBuilder& geometryAs);

    class BottomGeometryAs;
    struct BottomGeometryAsBuilder
    {
        BuildAccelerationStructureFlag flags;
        View<BottomGeometryAs>         as;
        View<Buffer>                   scratchBuffer;
        CSpan<GeometryAsBuilder>       geometries;
    };

    uint64_t getAccelerationStructureSize(BuildAccelerationStructureFlag        flags,
                                          const std::vector<GeometryAsBuilder>& geometries);

    class BottomGeometryAs
    {
      public:
        BottomGeometryAs(View<Device> device, std::vector<GeometryAsBuilder> geometries,
                         BuildAccelerationStructureFlag flags = BuildAccelerationStructureFlag::PreferFastBuild);
        BottomGeometryAs(View<Device> device, GeometryAsBuilder geometry,
                         BuildAccelerationStructureFlag flags = BuildAccelerationStructureFlag::PreferFastBuild);

        BottomGeometryAs(const BottomGeometryAs&)            = delete;
        BottomGeometryAs& operator=(const BottomGeometryAs&) = delete;

        BottomGeometryAs(BottomGeometryAs&&) noexcept;
        BottomGeometryAs& operator=(BottomGeometryAs&&) noexcept;

        ~BottomGeometryAs();

        inline uint64_t                   getSize() const;
        inline View<Buffer>               getBuffer() const;
        inline VkAccelerationStructureKHR getHandle() const;

      private:
        VkAccelerationStructureKHR     m_handle = VK_NULL_HANDLE;
        View<Device>                   m_device;
        std::vector<GeometryAsBuilder> m_geometries;
        Buffer                         m_buffer;
        uint64_t                       m_deviceAddress;
        uint64_t                       m_size;
    };

} // namespace vzt

#include "vzt/Vulkan/Raytracing/AccelerationStructure.inl"

#endif // VZT_VULKAN_RAYTRACING_ACCELERATIONSTRUCTURE_HPP
