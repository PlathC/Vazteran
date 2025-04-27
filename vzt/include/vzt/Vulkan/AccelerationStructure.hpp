#ifndef VZT_VULKAN_RAYTRACING_ACCELERATIONSTRUCTURE_HPP
#define VZT_VULKAN_RAYTRACING_ACCELERATIONSTRUCTURE_HPP

#include <variant>

#include "vzt/Core/Math.hpp"
#include "vzt/Vulkan/Buffer.hpp"
#include "vzt/Vulkan/DeviceObject.hpp"
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

    enum class BuildAccelerationStructureFlag : uint32_t
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

        Optional<BufferCSpan> transformBuffer = {};
    };

    using AabbPositions = VkAabbPositionsKHR;
    struct AsAabbs
    {
        BufferCSpan aabbs; // Must contain AabbPositions
        std::size_t stride = 0;
    };

    enum class GeometryInstanceFlag : uint8_t
    {
        TriangleFacingCullDisable = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
        TriangleFlibFacing        = VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR,
        ForceOpaque               = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR,
        ForceNoOpaque             = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR,

        // Provided by VK_EXT_opacity_micromap
        ForceOpacityMicroMap2State = VK_GEOMETRY_INSTANCE_FORCE_OPACITY_MICROMAP_2_STATE_EXT,

        // Provided by VK_EXT_opacity_micromap
        DisableOpacityMicromaps       = VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_EXT,
        TriangleFrontCounterClockwise = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(GeometryInstanceFlag, VkGeometryInstanceFlagsKHR)
    VZT_DEFINE_BITWISE_FUNCTIONS(GeometryInstanceFlag)

    struct AsInstance
    {
        uint64_t deviceAddress;
        uint32_t count;
    };

    using AsGeometry = std::variant<AsTriangles, AsAabbs, AsInstance>;

    struct GeometryAsBuilder
    {
        AsGeometry   geometry;
        GeometryFlag flags = GeometryFlag::Opaque;
    };
    VkAccelerationStructureGeometryKHR toVulkan(const GeometryAsBuilder& geometryAs);

    class AccelerationStructure;
    struct AccelerationStructureBuilder
    {
        BuildAccelerationStructureFlag flags;
        View<AccelerationStructure>    as;
        View<Buffer>                   scratchBuffer;
        uint32_t                       scratchBufferMinAlignment = 1;
    };

    uint64_t getAccelerationStructureSize(BuildAccelerationStructureFlag        flags,
                                          const std::vector<GeometryAsBuilder>& geometries);

    enum class AccelerationStructureType
    {
        TopLevel    = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        BottomLevel = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        Generic     = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(AccelerationStructureType, VkAccelerationStructureTypeKHR)

    class AccelerationStructure : public DeviceObject<VkAccelerationStructureKHR>
    {
      public:
        AccelerationStructure() = default;

        AccelerationStructure(View<Device> device, std::vector<GeometryAsBuilder> geometries,
                              AccelerationStructureType type);
        AccelerationStructure(View<Device> device, GeometryAsBuilder geometry, AccelerationStructureType type);

        AccelerationStructure(const AccelerationStructure&)            = delete;
        AccelerationStructure& operator=(const AccelerationStructure&) = delete;

        AccelerationStructure(AccelerationStructure&&) noexcept;
        AccelerationStructure& operator=(AccelerationStructure&&) noexcept;

        ~AccelerationStructure() override;

        inline AccelerationStructureType             getType() const;
        inline uint64_t                              getSize() const;
        inline uint64_t                              getScratchBufferSize() const;
        inline View<Buffer>                          getBuffer() const;
        inline const std::vector<GeometryAsBuilder>& getGeometries() const;
        inline uint64_t                              getDeviceAddress() const;

      private:
        std::vector<GeometryAsBuilder> m_geometries;
        AccelerationStructureType      m_type;
        uint32_t                       m_maxPrimitiveCount{};
        Buffer                         m_buffer;
        uint64_t                       m_deviceAddress{};
        uint64_t                       m_size{};
        uint64_t                       m_scratchBufferSize{};
    };

} // namespace vzt

#include "vzt/Vulkan/AccelerationStructure.inl"

#endif // VZT_VULKAN_RAYTRACING_ACCELERATIONSTRUCTURE_HPP
