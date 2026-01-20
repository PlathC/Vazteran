#ifndef VZT_VULKAN_RAYTRACING_ACCELERATION_STRUCTURE_HPP
#define VZT_VULKAN_RAYTRACING_ACCELERATION_STRUCTURE_HPP

#include <variant>

#include "vzt/core/math.hpp"
#include "vzt/vulkan/buffer.hpp"
#include "vzt/vulkan/device.hpp"
#include "vzt/vulkan/type.hpp"

namespace vzt
{
    class Device;

    struct AccelerationStructureTriangles
    {
        Format      vertexFormat;
        BufferCSpan vertexBuffer;
        std::size_t vertexStride;
        std::size_t maxVertex;
        BufferCSpan indexBuffer;

        Optional<BufferCSpan> transformBuffer = {};
    };

    using AabbPositions = VkAabbPositionsKHR;
    struct AccelerationStructureAabbs
    {
        BufferCSpan aabbs; // Must contain AabbPositions
        std::size_t stride = 0;
    };

    struct AccelerationStructureInstance
    {
        uint64_t deviceAddress;
        uint32_t count;
    };

    using AccelerationStructureGeometry =
        std::variant<AccelerationStructureTriangles, AccelerationStructureAabbs, AccelerationStructureInstance>;

    struct GeometryAccelerationStructureBuilder
    {
        AccelerationStructureGeometry geometry;
        GeometryFlag                  flags = GeometryFlag::Opaque;
    };
    VkAccelerationStructureGeometryKHR toVulkan(const GeometryAccelerationStructureBuilder& geometryAs);

    class AccelerationStructure;
    struct AccelerationStructureBuilder
    {
        BuildAccelerationStructureFlag flags;
        View<AccelerationStructure>    as;
        View<Buffer>                   scratchBuffer;
        uint32_t                       scratchBufferMinAlignment = 1;
    };

    uint64_t getAccelerationStructureSize(BuildAccelerationStructureFlag                           flags,
                                          const std::vector<GeometryAccelerationStructureBuilder>& geometries);

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

        AccelerationStructure(View<Device> device, std::vector<GeometryAccelerationStructureBuilder> geometries,
                              AccelerationStructureType type);
        AccelerationStructure(View<Device> device, GeometryAccelerationStructureBuilder geometry,
                              AccelerationStructureType type);

        AccelerationStructure(const AccelerationStructure&)            = delete;
        AccelerationStructure& operator=(const AccelerationStructure&) = delete;

        AccelerationStructure(AccelerationStructure&&) noexcept;
        AccelerationStructure& operator=(AccelerationStructure&&) noexcept;

        ~AccelerationStructure() override;

        inline AccelerationStructureType                                getType() const;
        inline uint64_t                                                 getSize() const;
        inline uint64_t                                                 getScratchBufferSize() const;
        inline View<Buffer>                                             getBuffer() const;
        inline const std::vector<GeometryAccelerationStructureBuilder>& getGeometries() const;
        inline uint64_t                                                 getDeviceAddress() const;

      private:
        std::vector<GeometryAccelerationStructureBuilder> m_geometries;
        AccelerationStructureType                         m_type;
        uint32_t                                          m_maxPrimitiveCount{};
        Buffer                                            m_buffer;
        uint64_t                                          m_deviceAddress{};
        uint64_t                                          m_size{};
        uint64_t                                          m_scratchBufferSize{};
    };
} // namespace vzt

#include "vzt/vulkan/acceleration_structure.inl"

#endif // VZT_VULKAN_RAYTRACING_ACCELERATION_STRUCTURE_HPP
